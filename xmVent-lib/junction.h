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

#ifndef XMVENTJUNCTION_H
#define XMVENTJUNCTION_H

#include "xmvent-global.h"

#include <QObject>
#include <QString>
#include <QVector3D>

class XMVENTSHARED_EXPORT XMVentJunction : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString id READ id WRITE setId )
    Q_PROPERTY( QVector3D point READ point WRITE setPoint )
    Q_PROPERTY( bool surface READ isSurface WRITE setSurface )

protected:
    QString m_id;
    QVector3D m_point;
    bool m_surface;

public:
    float pressure;             // TODO:AW: make this property
    bool referencePressure;     // TODO:AW: make this property

    explicit XMVentJunction( QObject *parent = 0 );

    //void glVertex() const;

    QString id() const;
    const QVector3D& point() const;
    QVector3D& point();         // TODO:AW: changes can occur outside setPoint(...)
    bool isSurface() const;

signals:

public slots:
    void setId( const QString& id );
    void setPoint( const QVector3D& point );
    void setSurface( bool surface );
};

#endif // XMVENTJUNCTION_H
