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

#include "branch.h"

XMVentBranch::XMVentBranch(QObject *parent) :
    QObject(parent)
{
    m_fromId = -1;
    m_toId = -1;
    m_resistance = 0.;
    m_n = 2.;
}


QString XMVentBranch::id() const
{
    return m_id;
}


void XMVentBranch::setId( const QString& id )
{
    m_id = id;
}


int XMVentBranch::fromId() const
{
    return m_fromId;
}


void XMVentBranch::setFromId( int fromId )
{
    m_fromId = fromId;
}


int XMVentBranch::toId() const
{
    return m_toId;
}


void XMVentBranch::setToId( int toId )
{
    m_toId = toId;
}

float XMVentBranch::resistance() const
{
    return m_resistance;
}


void XMVentBranch::setResistance( float resistance )
{
    m_resistance = resistance;
}


float XMVentBranch::n() const
{
    return m_n;
}


void XMVentBranch::setN( float n )
{
    m_n = n;
}
