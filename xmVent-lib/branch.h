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

#ifndef XMVENTBRANCH_H
#define XMVENTBRANCH_H

#include "xmvent-global.h"


#include <QObject>
#include <QString>

class XMVENTSHARED_EXPORT XMVentBranch : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString id READ id WRITE setId )
    Q_PROPERTY( int fromId READ fromId WRITE setFromId )
    Q_PROPERTY( int toId READ toId WRITE setToId )
    Q_PROPERTY( float resistance READ resistance WRITE setResistance )
    Q_PROPERTY( float n READ n WRITE setN )

protected:
    QString m_id;
    int m_fromId, m_toId;
    float m_resistance;
    float m_n;

public:

    explicit XMVentBranch(QObject *parent = 0);

    QString id() const;
    int fromId() const;
    int toId() const;
    float resistance() const;
    float n() const;

signals:

public slots:
    void setId( const QString& id );
    void setFromId( int fromId );
    void setToId( int toId );
    void setResistance( float resistance );
    void setN( float n );

};

#endif // XMVENTBRANCH_H
