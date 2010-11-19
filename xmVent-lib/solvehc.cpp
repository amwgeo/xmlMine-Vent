/*
 *  Copyright (C) 2010 Andrew Wilson.
 *  All rights reserved.
 *  Contact email: amwgeo@gmail.com
 *
 *  This file is part of xmlMine-Vent
 *
 *  xmlMine-Vent is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation, either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  xmlMine-Vent is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General
 *  Public License along with xmlMine-Vent.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "solvehc.h"

#include "network.h"
#include "branch.h"
#include "junction.h"
#include "fan.h"

#include <QDebug>
#include <QtAlgorithms>
#include <QScriptEngine>

#include <cmath>
#include <climits>
using namespace std;

// TODO:AW: presize arrays where possible


/****************  (exerpt from McPherson Section 7.3.2 Numerical Methods) ****************

    The Hardy Cross procedure may now be summarized as follows:

    (a) Establish a network schematic and choose at least (b-j+l) closed meshes such that all
        branches are represented (section 7.3.1.2). [b = number of branches and j = number of junctions
        in the network.]  Convergence to a balanced solution will be improved by ensuring that each high
        resistance branch is included in only one mesh.

    (b) Make an initial estimate of airflow, Qa, for each branch, ensuring that Kirchhoff I is obeyed.

    (c) Traverse one mesh and calculate the mesh correction factor ∆Qm from equation (7.35) or (7.36).

                    (7.35)  ∆Qm = −Σ(RQa Qan−1 − pf − nvp) / Σ(nRQan−1 +Sf +Snv)

                    (7.36)  ∆Qm = −Σ(RQa Qa −pf −nvp) / Σ(2RQa +Sf +Snv)

    (d) Traverse the same mesh, in the same direction, and adjust each of the contained airflows by
        the amount ∆Qm.

    (e) Repeat steps (c) and (d) for every mesh in the network.

    (f) Repeat steps (c), (d) and (e) until Kirchhoff II is satisfied to an acceptable degree of
        accuracy, i.e. until all values of − Σ(RQi Qin−1 − pf − nvp) are close to zero, where Qi are the
        current values of airflow.

 ******************************************************************************************/

/// walk through all dependent branches and sum resistance
float branchDependentWalk( int branchId,  const QList<XMVentBranch*>& branches,
                           const QMultiMap<int,QPair<int,int> >& adj, QList<int>& branchAvoid )
{
    const XMVentBranch* branch = branches[branchId];
    float resistance = branch->resistance();

    // avoid current branch
    branchAvoid.append( branchId );

    // calculate super-branch resistance
    for( int i=0; i<2; i++ ) {  // search from-node then to-node
        int lastBranchId = branchId;
        int nodeId = ( i==0 ? branch->fromId() : branch->toId() );

        // step until end of independent super-branch
        QList<QPair<int,int> > step = adj.values( nodeId );
        while( 2 == step.size() ) {
            int stepId = ( step[0].second == lastBranchId ? 1 : 0 );
            int stepNodeId = step[stepId].first;
            int stepBranchId = step[stepId].second;

            //Q_ASSERT( !branchAvoid.contains( stepBranchId ) );      // branch should not be in branchAvoid
            resistance += branches[ stepBranchId ]->resistance();
            branchAvoid.append( stepBranchId );

            // next step
            nodeId = stepNodeId;
            lastBranchId = stepBranchId;
            step = adj.values( nodeId );
        }
    }

    return resistance;
}


/// create a list of high-resistance independent seed branches
QList<int> branchPriority( const QList<XMVentBranch*>& branches,
                    const QMultiMap<int,QPair<int,int> >& adj,
                    QList<int> branchAvoid = QList<int>() )
{
    // Insert sort into map (resistance, branchId)
    QMultiMap<double, int> map;
    QList<class XMVentBranch*>::const_iterator it;
    int branchId = 0;
    for( it = branches.begin(); it != branches.end(); it++, branchId++ ) {
        if( !branchAvoid.contains( branchId ) ) {
            // calculate total resistance for all dependent branches
            float resistance = branchDependentWalk( branchId, branches, adj, branchAvoid );

            // add total branch to map
            map.insertMulti( resistance, branchId );
            //qDebug() << "Branch" << branchId << ": total resistance =" << resistance;
        }
    }

    return map.values();
}


/// create surface branches to complete network
void addSurfaceJunctions( XMVentNetwork* net )
{
    bool first = true;
    QList<class XMVentJunction*>::const_iterator it;
    int junctionId = 0;
    int firstSurfaceId;
    for( it = net->m_junction.begin(); it != net->m_junction.end(); it++, junctionId++ ) {
        XMVentJunction* junction = *it;
        if( junction->isSurface() ) { // test if surface junction
            if( first ) {
                firstSurfaceId = junctionId;
                first = false;
            } else { // add a surface branch
                XMVentBranch* branch = new XMVentBranch( net );
                branch->setResistance( 0. );
                branch->setFromId( firstSurfaceId );
                branch->setToId( junctionId );
                net->m_branch.append( branch );
            }
        }
    }
}


/// create an Adjacency map for nodes including branch numbers
QMultiMap<int,QPair<int,int> > nodeAdjacency( const QList<class XMVentBranch*>& branches )
{
    QMultiMap<int,QPair<int,int> > nodeAdjency;

    int branchId;
    QList<XMVentBranch*>::ConstIterator it;
    for( it = branches.begin(), branchId = 0; it != branches.end(); it++, branchId++ ) {

        XMVentBranch* branch = *it;

        // TODO: struct adjacentStep { nodeId, branchId, resitance, branchDirection } ?
        nodeAdjency.insertMulti( branch->fromId(), QPair<int,int>(branch->toId(), branchId) );
        nodeAdjency.insertMulti( branch->toId(), QPair<int,int>(branch->fromId(), branchId) );
    }

    return nodeAdjency;
}


// TODO:AW: MultiMap -> MultiHash, List -> Set ?!
/// walk network to find the smallest loop starting with startBranchId and avoiding branchAvoid.
/// algorithm based on a LIFO stack instead of a recursive algorithm for efficiency
QList<int> findSmallestClosedWalk( int startBranchId, const XMVentNetwork* net,
    const QMultiMap<int,QPair<int,int> >& adj, const QList<int> branchAvoid = QList<int>() )
{
    QList<int> branchList, smallestWalk;
    QMap<int, int> nodeDistance;                // TODO: does this superseed the avoidBranch or otherwise?

    // Seed the stack
    QList<QList<QPair<int,int> > > stack;
    stack.append( adj.values( net->m_branch[ startBranchId ]->toId() ) );
    branchList.append( startBranchId );
    const int nodeIdStart = net->m_branch[ startBranchId ]->fromId();
    // TODO:AW: nodeDistance here?

    // Process the stack
    while( stack.count() > 0 ) {
        if( stack.last().count() == 0 ) {
            // step back in walk by droping last item from stack
            stack.pop_back();
            branchList.pop_back();

        } else {
            // get next step information from stack
            QPair<int,int> pair = stack.last().last();
            stack.last().pop_back();
            int nodeId = pair.first;
            int branchId = pair.second;

            // skip branch if already used in branchList or in branchAvoid
            if( !branchList.contains( branchId ) && !branchAvoid.contains( branchId ) ) {
                if( nodeId == nodeIdStart ) {
                    // loop detected
                    smallestWalk = branchList;
                    smallestWalk.append( branchId );
                } else {
                    // skip if we've already visited this point faster.
                    if( nodeDistance.value( nodeId, INT_MAX ) >= stack.count() ) {

                        // do next step only if smaller walk is still possible
                        if( smallestWalk.count() == 0 || (branchList.count()+2) < smallestWalk.count() ) {
                            // append new stack level
                            stack.append( adj.values( nodeId ) );
                            branchList.append( branchId );
                            nodeDistance[ nodeId ] = stack.count();
                        }
                    }
                }
            }
        }
    }

    return smallestWalk;
}


QList<QList<float> > meshDirectionCoefficients( const QList<QList<int> >& meshList,
                                  const QList<XMVentBranch*>& branches )
{
    QList<QList<float> > coeff;

    QList<QList<int> >::const_iterator itMesh;
    for( itMesh = meshList.begin(); itMesh != meshList.end(); itMesh++ ) {
        coeff.push_back( QList<float>() );    // push an empty list
        QList<float>& meshCoeff( coeff.last() );

        // populate that list
        int lastNodeId;
        bool first = true;
        QList<int>::const_iterator itBranch;
        for( itBranch = (*itMesh).begin(); itBranch != (*itMesh).end(); itBranch++) {
            const XMVentBranch* pBranch = branches[(*itBranch)];
            if( first ) {
                lastNodeId = pBranch->toId();
                meshCoeff.append( 1. );
                first = false;
            } else {
                bool forward = (pBranch->fromId() == lastNodeId);
                meshCoeff.append( forward ? 1. : -1. );
                lastNodeId = ( forward ? pBranch->toId() : pBranch->fromId() );
            }
        }
    }

    return coeff;
}


/// create network meshes
void XMVentSolveHC::createMesh()
{
    m_meshList.empty();
    QMultiMap<int,QPair<int,int> > nodeAdj = nodeAdjacency( m_ventNet->m_branch );

    QList<int> branchFixedFlow = m_ventNet->m_fixedFlow.keys();

    // Walk through all fixed flow branches
    QList<int> branchAvoid;
    QList<int>::const_iterator itBranchId;
    for( itBranchId = branchFixedFlow.begin(); itBranchId != branchFixedFlow.end(); itBranchId++ ) {
        branchDependentWalk( *itBranchId, m_ventNet->m_branch, nodeAdj, branchAvoid );
    }

    // create seed priority list
    int nMeshes = m_ventNet->m_branch.count() - m_ventNet->m_junction.count() + 1;
    int nMeshBalance = nMeshes - branchFixedFlow.count();
    QList<int> branchList = branchPriority( m_ventNet->m_branch, nodeAdj, branchAvoid );

    branchAvoid = branchFixedFlow;
    for( int i = 0; i < nMeshes; i++ ) {
        int branchId;
        if( i < nMeshBalance ) {    // non-fixed flow meshes
            branchId = branchList.last();
        } else {    // fixed-flow meshes
            branchId = branchFixedFlow[ i - nMeshBalance ];
            // unblock branch
            branchAvoid.removeAll( branchId );
        }

        // find shortest closed walk around graph
        m_meshList.append( findSmallestClosedWalk( branchId, m_ventNet, nodeAdj, branchAvoid ) );

        // remove all newly used branches in walk from branchList
        QList<int>::const_iterator it;
        for( it = m_meshList.last().begin(); it != m_meshList.last().end(); it++ ) {
            branchList.removeAll( *it );
        }
    }

    m_meshCoeff = meshDirectionCoefficients( m_meshList, m_ventNet->m_branch );
}


void XMVentSolveHC::flowInitialize()
{
    // create and initialize flow values to zero
    m_flowList.empty();
    int nBranches = m_ventNet->m_branch.count();
    m_flowList.reserve( nBranches );
    for( int i = 0; i < nBranches; i++ ) {
        m_flowList.append( 0. );
    }

    // initialize flows using mesh loops
    int nMeshBalanced = m_meshList.count() - m_ventNet->m_fixedFlow.count();
    QMap<int, float>::const_iterator itFixedFlow = m_ventNet->m_fixedFlow.begin();
    for( int i = 0; i < m_meshList.count(); i++ ) {
        // initialize each branch with 1 or specified fixed flow if applicable
        float fixed = 1.;
        int iFixedFlow = i - nMeshBalanced;
        if( iFixedFlow >= 0 ) {
            fixed = (itFixedFlow++).value();
        }

        // for all branches in mesh: flow[branch] += fixed * coeff[branch]
        QList<int>::const_iterator itBranch;
        QList<float>::const_iterator itCoeff = m_meshCoeff[i].begin();
        for( itBranch = m_meshList[i].begin(); itBranch != m_meshList[i].end(); itBranch++, itCoeff++ ) {
            m_flowList[*itBranch] += fixed * (*itCoeff);
        }
    }
}


struct MeshAdjust {
    float pressure;
    float slope;
};


/// calculate mesh pressure imbalance and slope (dP/dQ) for correction
MeshAdjust pressureAdjustBranch( const QList<float>& flow,
                                 const QList<int>& mesh, const QList<float>& coeff,
                                 const QList<class XMVentBranch*>& branchList,
                                 const QMap<int,class XMVentFan*>& fanList )
{
    MeshAdjust adj;
    adj.pressure = 0.;
    adj.slope = 0.;

    for( int j = 0; j < mesh.size(); j++ ) {    // for each branch in mesh
        int branchId = mesh[ j ];
        const XMVentBranch* branch = branchList[ branchId ];
        float n = branch->n();
        float q = coeff[ j ] * flow[ branchId ];    // signed flow relative to mesh direction ?
        float rq = pow( fabs(q), n - 1.f) * branch->resistance();
        adj.pressure += rq * q; // pressure += fsp + nvp
        adj.slope += n * rq;   // pressure += slope(fsp) + slope(nvp)

        // add fan static pressure
        const XMVentFan* fan = fanList.value( branchId, 0 );
        if( fan ) {
            // TODO:AW: variable pressure fans
            adj.pressure -= coeff[j] * fan->fixedPressure();
        }
    }

    return adj;
}


/// solve for next Hardy-Cross iteration step
// TODO:AW: test over relaxation 1 < lambda < 2 to accelerate convergence
float ventSolveHCIterate( QList<float>& flow, const QList<class XMVentBranch*>& branchList,
                          const QList<QList<int> >& meshList, const QList<QList<float> >& meshCoeff,
                          const QMap<int,class XMVentFan*>& fanList, int nMeshBalanced,
                          float lambda )
{
    float meshCorrection = 0;

    for( int i = 0; i < nMeshBalanced; i++ ) {  // for each mesh, but not fixed-flow meshes
        QList<int> mesh = meshList[i];
        QList<float> coeff = meshCoeff[i];
        // calculate correction
        MeshAdjust adj = pressureAdjustBranch( flow, mesh, coeff, branchList, fanList );

        meshCorrection += abs(adj.pressure);

        // apply correction
        if( adj.slope != 0. ) { // TODO:AW: is this okay or should it be a fuzzy test for "too small"?
            float meshFlowCorrection = - adj.pressure / adj.slope * lambda;
            for( int j = 0; j < mesh.size(); j++ ) {
                // correction adjusted for branch direction
                flow[ mesh[ j ] ] += coeff[ j ] * meshFlowCorrection;
            }
        }
    }

    return meshCorrection;
}


/// Hardy-Cross iterative solution
/// tolerance - sum of absolute mesh pressure error in pascals?
bool XMVentSolveHC::solve( float meshCorrectionTolerance, int iterationMax, float lambda )
{
    // initialize if it has not already been done
    if( m_meshList.count() == 0 ) {
        initialize();
    }

    const int nMeshBalanced = m_meshList.count() - m_ventNet->m_fixedFlow.count();

    // Iterate to balance the network until tolerance achieved or maximum iterations
    float meshCorrection = +INFINITY;
    int i;
    for( i = 0; (i < iterationMax) && (meshCorrection > meshCorrectionTolerance) ; i++ ) {
        meshCorrection = ventSolveHCIterate( m_flowList, m_ventNet->m_branch, m_meshList,
                                             m_meshCoeff, m_ventNet->m_fanList, nMeshBalanced, lambda );

        //qDebug() << "Iteration" << i << "meshCorrection:" << meshCorrection;
    }

    if( i != iterationMax ) {
        qDebug() << "Solution found after iteration" << i;
    } else {
        qDebug() << "Did not achieve convergence criteria after iteration" << i;
    }

    return i == iterationMax;
}


/// Hardy-Cross iterative solution with Aitken convergence acceleration
/// tolerance - sum of absolute pressure error in pascals?
/*void ventSolveHC_Aitken( QList<float>& flow, const XMVentNetwork* net, const QList<QList<int> >& meshList,
                  const QList<QList<float> >& meshCoeff, float meshCorrectionTolerance = 0.1,
                  int iterationMax = 5000 )
{
    const int nMeshBalanced = meshList.count() - net->fixedFlow.count();

    QList<float> flowSet[3];

    // Iterate to balance the network until tolerance achieved or maximum iterations
    int i;
    float meshCorrection = +INFINITY;
    for( i = 0; (i < iterationMax) && (meshCorrection > meshCorrectionTolerance) ; i++ ) {
        QList<float>& f0( flowSet[ i % 3 ] );
        QList<float>& f1( flowSet[ ( i + 1 ) % 3 ] );
        QList<float>& f2( flowSet[ ( i + 2 ) % 3 ] );

        // set current working flow to last (or initial) flow vector
        f0 = (i == 0) ? flow : f2;

        meshCorrection = ventSolveHCIterate( f0, net, meshList, meshCoeff, nMeshBalanced );

        qDebug() << "Iteration" << i << "flow:" << f0;

        // convergence acceleration using Aitken's delta-squared process
        if( i > 1 ) {
            for( int j = 0; j < f0.count(); j++ ) {
                // (f0*f1-f2*f2)/(f0-2*f2+f1)
                float denom = f0[j] - 2.*f2[j] + f1[j];
                if( fabs(denom) > 1e-10 ) {
                    f0[j] = (f0[j]*f1[j] - f2[j]*f2[j]) / denom;
                }
            }
            qDebug() << "Aitken   " << i << "flow:" << f0;
        }

    }

    flow = flowSet[ --i % 3 ];
}//*/



/// Ventilation Solver valid so long as mesh does not change.
XMVentSolveHC::XMVentSolveHC( QObject* parent, XMVentNetwork* ventNet ) : QObject( parent ), m_ventNet( ventNet )
{
}


void XMVentSolveHC::initialize()
{
    // reset all structures
    m_meshList.empty();
    m_meshCoeff.empty();
    m_flowList.empty();

    // add surface junctions, just like the function name suggests
    // TODO:AW: delete old surface junctions?
    addSurfaceJunctions( m_ventNet );

    // find mesh and mesh direction coefficients
    createMesh();
    qDebug() << "Meshes: " << m_meshList;
    qDebug() << "MeshCoeff: " << m_meshCoeff;

    // initialize flow
    flowInitialize();
    qDebug() << "Initialized flow:" << m_flowList;
}


QList<float> XMVentSolveHC::getFlow() const
{
    return m_flowList;
}


void XMVentSolveHC::setFlow( const QList<float>& flow )
{
    m_flowList = flow;
}

/// returns a list of booster fsp [Pa] (positive); or resistance [Ns2/m8] (negative)
QList<float> XMVentSolveHC::fixedFlowPressure() const
{
    QList<float> fixedFlowPressure;

    int nMeshBalance = m_meshList.count() - m_ventNet->m_fixedFlow.count();
    QMap<int, float>::const_iterator itFixedFlow = m_ventNet->m_fixedFlow.begin();
    for( int i = nMeshBalance; i < m_meshList.count(); i++, itFixedFlow++ ) { // for each fixed-flow branch
        QList<int> mesh = m_meshList[i];
        QList<float> coeff = m_meshCoeff[i];
        // calculate correction
        MeshAdjust adj = pressureAdjustBranch( m_flowList, mesh, coeff, m_ventNet->m_branch, m_ventNet->m_fanList );

        if( adj.pressure < 0 ) {
            // calculate regulator resistance
            float q = itFixedFlow.value();
            adj.pressure /= q * q;
        }

        fixedFlowPressure.append( adj.pressure );
    }

    return fixedFlowPressure;
}
