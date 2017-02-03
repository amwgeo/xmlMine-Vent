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

#include "network.h"

#include <QtXml>
#include <QMap>
#include <QQmlEngine>


#include "junction.h"
#include "branch.h"
#include "fan.h"

Q_DECLARE_METATYPE(QList<float>)


class XMVentNetworkParser: public QXmlDefaultHandler
{
protected:
    QString scriptCode;
    bool insideScript;
    XMVentNetwork& m_ventNet;
    QMap<QString, unsigned int> junctionMap;

    XMVentNetworkParser( XMVentNetwork& ventNet ) :
        m_ventNet( ventNet )
    {
        insideScript = false;
    }

public:
    bool startDocument() {
        m_ventNet.clear();
        return true;
    }

    bool endDocument() {
        return true;
    }

    bool addJunction( const QXmlAttributes& atts )
    {
        XMVentJunction* junction = new XMVentJunction( &m_ventNet );

        bool ok_x, ok_y, ok_z, ok_p = true;
        junction->point().setX( atts.value( "x" ).toFloat( &ok_x ) );
        junction->point().setY( atts.value( "y" ).toFloat( &ok_y ) );
        junction->point().setZ( atts.value( "z" ).toFloat( &ok_z ) );
        junction->setId( atts.value( "id" ) );
        junction->setSurface( 0 == atts.value( "surface" ).compare("true", Qt::CaseInsensitive) );
        if( -1 != atts.index("pressure") ) {
            junction->pressure = atts.value("pressure").toFloat( &ok_p );
            junction->referencePressure = true;
        }
        if( ok_x && ok_y && ok_z && ok_p && !junction->id().isNull() ) {
            junctionMap.insert( junction->id(), m_ventNet.m_junction.count() );
            m_ventNet.m_junction.append( junction );
            return true;
        }
        delete junction;
        return false;
    }

    bool addBranchFan( int branchId, const QStringList& fanId ) {
        if( fanId.count() != 2 ) {
            return false;   // bad formatting
        }

        if( !fanId[0].isEmpty() ) {
            return false;           // TODO:AW: external fan definitions
        }

        QList<class XMVentFan*>::const_iterator it;
        for( it = m_ventNet.m_fanDefinition.begin(); it != m_ventNet.m_fanDefinition.end(); it++ ) {
            if( fanId[1] == (*it)->id() ) {
                m_ventNet.m_fanList.insert( branchId, *it );
                return true;
            }
        }

        return false;
    }

    bool addBranch( const QXmlAttributes& atts )
    {
        QString from( atts.value("from") );
        QString to( atts.value("to") );
        bool ok_r;
        float r = atts.value("resistance").toFloat( &ok_r );

        if( ok_r && junctionMap.contains(from) && junctionMap.contains(to) ) {
            XMVentBranch* branch = new XMVentBranch( &m_ventNet );
            branch->setResistance( r );
            branch->setFromId( junctionMap[ atts.value("from") ] );
            branch->setToId( junctionMap[ atts.value("to") ] );
            branch->setId( atts.value("id") );
            int branchId = m_ventNet.m_branch.count();
            m_ventNet.m_branch.append( branch );

            // fixed flow branch
            if( -1 != atts.index( "flow" ) ) {
                bool ok_flow;
                float flow = atts.value("flow").toFloat(&ok_flow);
                if( !ok_flow ) {
                    return false;
                }
                m_ventNet.m_fixedFlow.insert( m_ventNet.m_branch.count() - 1, flow );
            }

            // attach a fan
            if( -1 != atts.index( "fan" ) ) {
                return addBranchFan( branchId, atts.value( "fan" ).split( '#' ) );
            }
            return true;
        }

        return false;
    }

    bool addFan( const QXmlAttributes& atts )
    {
        XMVentFan* fan = new XMVentFan( &m_ventNet );
        fan->setId( atts.value( "id" ) );           // TODO:AW: make sure this is unique
        m_ventNet.m_fanDefinition.append( fan );
        if( -1 != atts.index("pressure") ) {
            bool ok_p;
            float p = atts.value( "pressure" ).toFloat( &ok_p );
            if( !ok_p ) {
                return false;
            }
            fan->setFixedPressure( p );
        }
        return true;
    }

    bool executeScript()
    {
        QJSEngine myEngine;
        static bool oneTime = true;

        if( oneTime ) {
            qmlRegisterType<XMVentFan>("com.mycompany.xmlmine", 1, 0, "XMVentFan");
            qmlRegisterType<QFile>("io.qt.core", 1, 0, "QFile");
            oneTime = false;
        }

        // TODO: import extensions in lieu of "qt.core"
        myEngine.installExtensions( QJSEngine::AllExtensions );

        QJSValue objectNet = myEngine.newQObject( &m_ventNet );
        myEngine.globalObject().setProperty( "net", objectNet );

        // QJSValue QJSEngine::newQObject(QObject *object)
        QJSValue objectSolver = myEngine.newQObject( &m_ventNet.m_solver );
        myEngine.globalObject().setProperty( "solver", objectSolver );

        QJSValue r = myEngine.evaluate( scriptCode );

        if( r.isError() ) {
            qDebug() << "ECMA Unhandled Exception:"
                << r.property("lineNumber").toInt()
                << ":" << r.toString();
            return false;
        } else {
            qDebug() << "ECMA executed without error";
        }

        return true;

    }

    bool startScript( const QXmlAttributes& atts )
    {
        if( atts.value("type") != "ECMAScript" ) {
            return false;
        }
        insideScript = true;
        scriptCode = "";
        return true;
    }

    bool characters( const QString& ch )
    {
        scriptCode.append( ch );
        return true;
    }

    bool endScript()
    {
        executeScript();
        insideScript = false;
        return true;
    }

    bool startElement(const QString& /*namespaceURI*/, const QString& /*localName*/, const QString& qName, const QXmlAttributes& atts )
    {
        // no elements allowed inside a script element.  CDATA counts as character data not an element.
        if( insideScript ) {
            return false;
        }

        if(qName == "junction") {
            return addJunction( atts );
        } else if( qName == "branch" ) {
            return addBranch( atts );
        } else if( qName == "fan" ) {
            return addFan( atts );
        } else if( qName == "script" ) {
            return startScript( atts );
        }

        // Ignore the unknown but continue to process XML
        return true;
    }

    bool endElement(const QString& /*namespaceURI*/, const QString& /*localName*/, const QString& qName)
    {
        if( qName == "script" ) {
            return endScript();
        }
        return true;
    }

    static void fromXml( QIODevice* dev, XMVentNetwork& ventNet )
    {
        // TODO:AW: error handle / return value
        XMVentNetworkParser handler( ventNet );
        QXmlInputSource source( dev );
        QXmlSimpleReader reader;
        reader.setContentHandler( &handler );
        reader.parse( source ) ;
    }

    static void fromXml( const QString& filename, XMVentNetwork& ventNet )
    {
        QFile file( filename );
        fromXml( &file, ventNet );
    }
};


XMVentNetwork::XMVentNetwork( QObject *parent ) :
    QObject(parent), m_solver( parent, this )
{
}


void XMVentNetwork::clear()
{
    QVector<XMVentJunction*>::iterator itJunct;
    for( itJunct = m_junction.begin(); itJunct != m_junction.end(); itJunct++ ) {
        delete *itJunct;
    }
    m_junction.clear();

    QVector<XMVentBranch*>::iterator itBranch;
    for( itBranch = m_branch.begin(); itBranch != m_branch.end(); itBranch++ ) {
        delete *itBranch;
    }
    m_branch.clear();

    QList<XMVentFan*>::iterator itFan;
    for( itFan = m_fanDefinition.begin(); itFan != m_fanDefinition.end(); itFan++ ) {
        delete *itFan;
    }
    m_fanDefinition.clear();

    m_fixedFlow.clear();
    m_fanList.clear();
}


void XMVentNetwork::fromXml( class QIODevice* dev )
{
    clear();
    XMVentNetworkParser::fromXml( dev, *this );
}


void XMVentNetwork::fromXml( const QString& filename )
{
    clear();
    XMVentNetworkParser::fromXml( filename, *this );
}




void XMVentNetwork::getLimits( float& x0, float& y0, float& z0, float& x1, float& y1, float& z1 ) const
{
    x0 = INFINITY;
    x1 = -INFINITY;
    y0 = INFINITY;
    y1 = -INFINITY;
    z0 = INFINITY;
    z1 = -INFINITY;
    QVector<XMVentJunction*>::const_iterator itJunct;
    for( itJunct = m_junction.begin(); itJunct != m_junction.end(); itJunct++ ) {
        float x = (*itJunct)->point().x();
        float y = (*itJunct)->point().y();
        float z = (*itJunct)->point().z();
        if( x0 > x ) x0 = x;
        if( x1 < x ) x1 = x;
        if( y0 > y ) y0 = y;
        if( y1 < y ) y1 = y;
        if( z0 > z ) z0 = z;
        if( z1 < z ) z1 = z;
    }
}


XMVentFan* XMVentNetwork::getFanDefinition( const QString& id )
{
    QList<class XMVentFan*>::iterator it;
    for( it = m_fanDefinition.begin(); it != m_fanDefinition.end(); it++ ) {
        if( (*it)->id() == id ) {
            return *it;
        }
    }
    return 0;
}


int XMVentNetwork::findBranchIndex( const QString& id ) const
{
    QVector<class XMVentBranch*>::const_iterator it;
    int i = 0;
    for( it = m_branch.begin(); it != m_branch.end(); it++, i++ ) {
        if( (*it)->id() == id ) {
            return i;
        }
    }
    return -1;
}




