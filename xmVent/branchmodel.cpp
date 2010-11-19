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

#include "branchmodel.h"

#include "xmVent-lib/network.h"
#include "xmVent-lib/branch.h"
#include "xmVent-lib/junction.h"

XMVentBranchModel::XMVentBranchModel( class XMVentNetwork& ventNet, QObject *parent ) :
        QAbstractListModel( parent ),
        m_ventNet( ventNet )
{
}


int XMVentBranchModel::rowCount( const QModelIndex& /*parent*/ ) const
{
    return m_ventNet.m_branch.count();
}


int XMVentBranchModel::columnCount( const QModelIndex& /*parent*/ ) const
{
    return 4;
}


QVariant XMVentBranchModel::data( const QModelIndex& index, int role ) const
{
    if( index.isValid() && role == Qt::DisplayRole && index.row() < m_ventNet.m_branch.count() ) {
        const XMVentBranch* branch = m_ventNet.m_branch[ index.row() ];
        switch( index.column() ) {
        case 0:
            return branch->id();
        case 1:
            return m_ventNet.m_junction[ branch->fromId() ]->id();
        case 2:
            return m_ventNet.m_junction[ branch->toId() ]->id();
        case 3:
            return branch->resistance();
        }
    }

    return QVariant();
}


QVariant XMVentBranchModel::headerData(
        int section, Qt::Orientation orientation, int role ) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch(section) {
        case 0:
            return tr("Id");
        case 1:
            return tr("From");
        case 2:
            return tr("To");
        case 3:
            return tr("Resistance");
        }
    }

    return tr("Branch %1").arg(section);
}
