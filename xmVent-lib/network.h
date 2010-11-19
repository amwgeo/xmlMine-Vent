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

#ifndef XMVENTNETWORK_H
#define XMVENTNETWORK_H

#include "xmvent-global.h"

#include <QObject>
#include <QMap>
#include "solvehc.h"

class XMVENTSHARED_EXPORT XMVentNetwork : public QObject
{
    Q_OBJECT

public:
    QList<class XMVentJunction*> m_junction;
    QList<class XMVentBranch*> m_branch;
    QList<class XMVentFan*> m_fanDefinition;
    QMap<int,float> m_fixedFlow;  // (branchId, fixed flow)
    QMap<int,class XMVentFan*> m_fanList;
    XMVentSolveHC m_solver;

    void clear();

    explicit XMVentNetwork( QObject *parent = 0 );

    void fromXml( class QIODevice* dev );
    Q_INVOKABLE void fromXml( const QString& filename );

    void getLimits( float& x0, float& y0, float& z0, float& x1, float& y1, float& z1 ) const;

    Q_INVOKABLE XMVentFan* getFanDefinition( const QString& id );
    Q_INVOKABLE int findBranchIndex( const QString& id ) const;

signals:

public slots:

};

#endif // XMVENTNETWORK_H
