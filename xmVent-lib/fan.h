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

#ifndef XMVENTFAN_H
#define XMVENTFAN_H

#include "xmvent-global.h"

#include <QObject>

class XMVENTSHARED_EXPORT XMVentFan : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString id READ id WRITE setId )
    Q_PROPERTY( float fixedPressure READ fixedPressure WRITE setFixedPressure )

protected:
    QString m_id;
    float m_fixedPressure;

public:
    explicit XMVentFan(QObject *parent = 0);

    QString id() const;
    void setId( const QString& id );

    float fixedPressure() const;
    void setFixedPressure( float fixedPressure );

signals:

public slots:

};

Q_DECLARE_METATYPE( XMVentFan* )

#endif // XMVENTFAN_H
