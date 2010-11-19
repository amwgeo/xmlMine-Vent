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

#include "fan.h"

//#include <cmath>
//using namespace std;

XMVentFan::XMVentFan(QObject *parent) :
    QObject(parent)
{
    m_fixedPressure = 0.;   //NAN;
}


QString XMVentFan::id() const
{
    return m_id;
}


void XMVentFan::setId( const QString& id )
{
    m_id = id;
}


float XMVentFan::fixedPressure() const
{
    return m_fixedPressure;
}


void XMVentFan::setFixedPressure( float fixedPressure )
{
    m_fixedPressure = fixedPressure;
}


void XMVentFan::scriptRegisterType( QScriptEngine *engine )
{
    qScriptRegisterMetaType( engine, XMVentFan::toScriptValue, XMVentFan::fromScriptValue );
}


QScriptValue XMVentFan::toScriptValue( QScriptEngine *engine, XMVentFan* const &in )
{
    return engine->newQObject(in);
}


void XMVentFan::fromScriptValue( const QScriptValue &object, XMVentFan* &out )
{
    out = qobject_cast<XMVentFan*>( object.toQObject() );
}

