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
//#include <QScriptEngine>

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
                           const QMultiHash<int,XMVentSolveHCStep>& adj, QList<int>& branchAvoid )
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
        QList<XMVentSolveHCStep> step = adj.values( nodeId );
        while( 2 == step.size() ) {
            int stepId = ( step[0].branchId == lastBranchId ? 1 : 0 );
            int stepNodeId = step[stepId].toNodeId;
            int stepBranchId = step[stepId].branchId;

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
                    const QMultiHash<int,XMVentSolveHCStep>& adj,
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
QMultiHash<int,XMVentSolveHCStep> nodeAdjacency( const QList<class XMVentBranch*>& branches )
{
    QMultiHash<int,XMVentSolveHCStep> nodeAdjacency;

    int branchId;
    XMVentSolveHCStep step;
    QList<XMVentBranch*>::ConstIterator itBranch;
    for( itBranch = branches.begin(), branchId = 0; itBranch != branches.end(); itBranch++, branchId++ ) {

        XMVentBranch* branch = *itBranch;
        step.branchId = branchId;

        // forward direction
        step.toNodeId = branch->toId();
        step.direction = 1.f;
        nodeAdjacency.insertMulti( branch->fromId(), step );

        // reverse direction
        step.toNodeId = branch->fromId();
        step.direction = -1.f;
        nodeAdjacency.insertMulti( branch->toId(), step );
    }

    qDebug() << "nodeAdjacency";
    QMultiHash<int,XMVentSolveHCStep>::ConstIterator itHash;
    for( itHash = nodeAdjacency.begin(); itHash != nodeAdjacency.end(); itHash++ ) {
        qDebug() << itHash.key() << "\t" << itHash.value().toNodeId << "\t" << itHash.value().branchId << "\t" << itHash.value().direction;
    }

    return nodeAdjacency;
}


/// return the coresponding index for a step with a given branchId
int findBranchId( const QList<XMVentSolveHCStep>& stepList, int branchId )
{
    int stepId;
    QList<XMVentSolveHCStep>::const_iterator itStep;
    for( itStep = stepList.begin(), stepId = 0; itStep != stepList.end(); itStep++, stepId++ ) {
        if( itStep->branchId == branchId ) {
            return stepId;
        }
    }

    // couldn't find branchId
    return -1;
}


/// walk network to find the smallest loop starting with startBranchId and avoiding branchAvoid.
/// algorithm based on a LIFO stack instead of a recursive algorithm for efficiency
QList<XMVentSolveHCStep> findSmallestClosedWalk( int startBranchId, const XMVentNetwork* net,
    const QMultiHash<int,XMVentSolveHCStep>& adj, const QList<int> branchAvoid = QList<int>() )
{
    QList<XMVentSolveHCStep> branchList, smallestWalk;
    QMap<int, int> nodeDistance;                // TODO: does this superseed the avoidBranch or otherwise?

    // Seed the stack
    QList<QList<XMVentSolveHCStep> > stack;
    stack.append( adj.values( net->m_branch[ startBranchId ]->toId() ) );
    XMVentSolveHCStep firstStep;
    firstStep.branchId = startBranchId;
    firstStep.direction = 1.f;
    firstStep.toNodeId = net->m_branch[ startBranchId ]->toId();
    branchList.append( firstStep );
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
            XMVentSolveHCStep step = stack.last().last();       // TODO:AW: rename pair to something useful
            stack.last().pop_back();
            //int nodeId = step.toNodeId;
            //int branchId = step.branchId;

            // skip branch if in branchAvoid
            if( branchAvoid.contains( step.branchId ) ) {
                continue;
            }

            // skip branch if already used in branchList or in branchAvoid

            if( -1 == findBranchId( branchList, step.branchId ) ) {
                if( step.toNodeId == nodeIdStart ) {
                    // loop detected
                    smallestWalk = branchList;
                    smallestWalk.append( step );
                } else {
                    // skip if we've already visited this point faster.
                    if( nodeDistance.value( step.toNodeId, INT_MAX ) >= stack.count() ) {

                        // do next step only if smaller walk is still possible
                        if( smallestWalk.count() == 0 || (branchList.count()+2) < smallestWalk.count() ) {
                            // append new stack level
                            stack.append( adj.values( step.toNodeId ) );
                            branchList.append( step );
                            nodeDistance[ step.toNodeId ] = stack.count();
                        }
                    }
                }
            }
        }
    }

    return smallestWalk;
}


/// create network meshes
void XMVentSolveHC::createMesh()
{
    m_meshList.empty();
    QMultiHash<int,XMVentSolveHCStep> nodeAdj = nodeAdjacency( m_ventNet->m_branch );

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
        QList<XMVentSolveHCStep>::const_iterator itStep;
        for( itStep = m_meshList.last().begin(); itStep != m_meshList.last().end(); itStep++ ) {
            branchList.removeAll( itStep->branchId );
        }
    }
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
        QList<XMVentSolveHCStep>::const_iterator itStep;
        for( itStep = m_meshList[i].begin(); itStep != m_meshList[i].end(); itStep++ ) {
            m_flowList[ itStep->branchId ] += fixed * itStep->direction;
        }
    }
}


struct MeshAdjust {
    float pressure;
    float slope;
};


/// calculate mesh pressure imbalance and slope (dP/dQ) for correction
MeshAdjust pressureAdjustBranch( const QList<float>& flow, const QList<XMVentSolveHCStep>& mesh,
                                 const QList<class XMVentBranch*>& branchList,
                                 const QMap<int,class XMVentFan*>& fanList )
{
    MeshAdjust adj;
    adj.pressure = 0.;
    adj.slope = 0.;

    for( int j = 0; j < mesh.size(); j++ ) {    // for each branch in mesh
        int branchId = mesh[ j ].branchId;
        const XMVentBranch* branch = branchList[ branchId ];
        float n = branch->n();
        float q = mesh[ j ].direction * flow[ branchId ];    // signed flow relative to mesh direction ?
        float rq = pow( fabs(q), n - 1.f) * branch->resistance();
        adj.pressure += rq * q; // pressure += fsp + nvp
        adj.slope += n * rq;   // pressure += slope(fsp) + slope(nvp)

        // add fan static pressure
        const XMVentFan* fan = fanList.value( branchId, 0 );
        if( fan ) {
            // TODO:AW: variable pressure fans
            adj.pressure -= mesh[ j ].direction * fan->fixedPressure();
        }
    }

    return adj;
}


/// solve for next Hardy-Cross iteration step
// TODO:AW: test over relaxation 1 < lambda < 2 to accelerate convergence
float ventSolveHCIterate( QList<float>& flow, const QList<class XMVentBranch*>& branchList,
                          const QList<QList<XMVentSolveHCStep> >& meshList, const QMap<int,class XMVentFan*>& fanList,
                          int nMeshBalanced, float lambda )
{
    float meshCorrection = 0;

    for( int i = 0; i < nMeshBalanced; i++ ) {  // for each mesh, but not fixed-flow meshes
        QList<XMVentSolveHCStep> mesh = meshList[i];
        // calculate correction
        MeshAdjust adjust = pressureAdjustBranch( flow, mesh, branchList, fanList );

        meshCorrection += abs( adjust.pressure );

        // apply correction
        if( adjust.slope != 0. ) { // TODO:AW: is this okay or should it be a fuzzy test for "too small"?
            float meshFlowCorrection = - adjust.pressure / adjust.slope * lambda;
            for( int j = 0; j < mesh.size(); j++ ) {
                // correction adjusted for branch direction
                flow[ mesh[ j ].branchId ] += mesh[ j ].direction * meshFlowCorrection;
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
                                             m_ventNet->m_fanList, nMeshBalanced, lambda );

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
    m_flowList.empty();

    // add surface junctions, just like the function name suggests
    // TODO:AW: delete old surface junctions?
    addSurfaceJunctions( m_ventNet );

    // find mesh and mesh direction coefficients
    createMesh();

    // initialize flow
    flowInitialize();
    qDebug() << "Initialized flow:" << m_flowList;
}


QVariantList XMVentSolveHC::getFlow() const
{
    QVariantList r;
    for( int i = m_flowList.size(); i>0; i-- ) {
        r.append( m_flowList[i] );
    }
    return r;
}


void XMVentSolveHC::setFlow( const QVariantList& flow )
{
    QList<float> r;
    for( int i = flow.size(); i>0; i-- ) {
        r.append( flow[i].toFloat() );
    }
    m_flowList = r;
}

/// returns a list of booster fsp [Pa] (positive); or resistance [Ns2/m8] (negative)
QVariantList XMVentSolveHC::fixedFlowPressure() const
{
    QVariantList fixedFlowPressure;

    int nMeshBalance = m_meshList.count() - m_ventNet->m_fixedFlow.count();
    QMap<int, float>::const_iterator itFixedFlow = m_ventNet->m_fixedFlow.begin();
    for( int i = nMeshBalance; i < m_meshList.count(); i++, itFixedFlow++ ) { // for each fixed-flow branch
        QList<XMVentSolveHCStep> mesh = m_meshList[i];
        // calculate correction
        MeshAdjust adj = pressureAdjustBranch( m_flowList, mesh, m_ventNet->m_branch, m_ventNet->m_fanList );

        if( adj.pressure < 0 ) {
            // calculate regulator resistance
            float q = itFixedFlow.value();
            adj.pressure /= q * q;
        }

        fixedFlowPressure.append( adj.pressure );
    }

    return fixedFlowPressure;
}
