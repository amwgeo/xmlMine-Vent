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

#include "junction.h"


XMVentJunction::XMVentJunction(QObject *parent) :
    QObject(parent)
{
    referencePressure = false;
    pressure = 0.;
    m_surface = false;
}


QString XMVentJunction::id() const
{
    return m_id;
}


void XMVentJunction::setId( const QString& id )
{
    m_id = id;
}


const QVector3D& XMVentJunction::point() const
{
    return m_point;
}


QVector3D& XMVentJunction::point()
{
    return m_point;
}


void XMVentJunction::setPoint( const QVector3D& point )
{
    m_point = point;
}


bool XMVentJunction::isSurface() const
{
    return m_surface;
}


void XMVentJunction::setSurface( bool surface )
{
    m_surface = surface;
}
