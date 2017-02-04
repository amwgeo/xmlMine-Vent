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

#include "junctionmodel.h"

#include "xmVent-lib/junction.h"
#include "xmVent-lib/network.h"

XMVentJunctionModel::XMVentJunctionModel( class XMVentNetwork& ventNet, QObject *parent ) :
        QAbstractListModel( parent ),
        m_ventNet( ventNet )
{
}

int XMVentJunctionModel::rowCount( const QModelIndex& /*parent*/ ) const
{
    return m_ventNet.m_junction.count();
}


int XMVentJunctionModel::columnCount( const QModelIndex& /*parent*/ ) const
{
    return 6;
}


QVariant XMVentJunctionModel::data( const QModelIndex& index, int role ) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole ) {
        return QVariant();
    }

    if( index.isValid() && index.row() < m_ventNet.m_junction.count() ) {
        const XMVentJunction* junction = m_ventNet.m_junction[ index.row() ];
        switch( index.column() ) {
        case 0:
            return junction->id();
        case 1:
            return float( junction->point().x() );
        case 2:
            return float( junction->point().y() );
        case 3:
            return float( junction->point().z() );
        case 4:
            return junction->isSurface();
        case 5:
            if( junction->referencePressure ) {
               return junction->pressure;
            }
        }
    }

    return QVariant();
}


QVariant XMVentJunctionModel::headerData(
        int section, Qt::Orientation orientation, int role ) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch(section) {
        case 0:
            return tr("id");
        case 1:
            return tr("x");
        case 2:
            return tr("y");
        case 3:
            return tr("z");
        case 4:
            return tr("Surface");
        case 5:
            return tr("Ref. Pressure");
        }
    }

    return tr("Junction %1").arg(section);
}


Qt::ItemFlags XMVentJunctionModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() )
        return Qt::ItemIsEnabled;

    if( index.column() <= 4 ) {
        return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
    }

    return QAbstractItemModel::flags( index );
}


bool XMVentJunctionModel::setData( const QModelIndex &index,
                              const QVariant &value, int role )
{
    if( index.isValid() && role == Qt::EditRole ) {
        XMVentJunction* junction = m_ventNet.m_junction[ index.row() ];

        bool ok = false;
        float newValue;

        switch( index.column() ) {
        case 0:
            // TODO: test for duplicate name ...
            junction->setId( value.toString() );
            break;
        case 1:
            newValue = value.toFloat( &ok );
            junction->point().setX( newValue );
            break;
        case 2:
            newValue = value.toFloat( &ok );
            junction->point().setY( newValue );
            break;
        case 3:
            newValue = value.toFloat( &ok );
            junction->point().setZ( newValue );
            break;
        case 4:
            junction->setSurface( value.toBool() );
            ok = true;
            break;
        }

        if( ok ) {
            emit dataChanged( index, index );
            return true;
        }
    }
    return false;
}


bool XMVentJunctionModel::insertRow( int row, const QModelIndex & /*parent*/ )
{
    // Only add junctions to the end of the list
    if( m_ventNet.m_junction.count() != row ) {
        return false;
    }

    XMVentJunction* junction = new XMVentJunction( &m_ventNet );
    m_ventNet.m_junction.append( junction );
    return true;
}

