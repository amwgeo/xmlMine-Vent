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

#ifndef XMVENTSOLVEHC_H
#define XMVENTSOLVEHC_H

#include "xmvent-global.h"

#include <QObject>
#include <QMultiMap>
#include <QVector>
#include <QVariantList>


/// Defines a single step while walking through the network
struct XMVentSolveHCStep {
    int toNodeId;
    int branchId;
    float direction;
};


/// Hardy-Cross Ventilation Network Solver
class XMVENTSHARED_EXPORT XMVentSolveHC : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QVariantList flow READ getFlow WRITE setFlow )

protected:
    class XMVentNetwork *m_ventNet;
    QList<QList<XMVentSolveHCStep> > m_meshList;

    void createMesh();
    void flowInitialize();

public:
    QList<float> m_flowList;

    explicit XMVentSolveHC( QObject* parent, class XMVentNetwork* ventNet );

    Q_INVOKABLE void initialize();
    Q_INVOKABLE bool solve( float meshCorrectionTolerance = 0.5f,
                            int iterationMax = 1000000,
                            float lambda = 1.5f );

    QVariantList getFlow() const;
    void setFlow( const QVariantList& flow );

    Q_INVOKABLE void clear();

    Q_INVOKABLE QVariantList fixedFlowPressure() const;
};


#endif // XMVENTSOLVEHC_H
