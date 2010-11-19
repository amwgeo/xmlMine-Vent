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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QContextMenuEvent>
//#include <QFileSystemModel>
#include "branchmodel.h"
#include "junctionmodel.h"
#include "glcamera.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

/*    QFileSystemModel *model = new QFileSystemModel( this );
    model->setRootPath( QDir::currentPath() );
    ui->treeViewBranches->setModel( model );
    ui->treeViewBranches->setRootIndex( model->index(QDir::currentPath()) ); //*/

    XMVentBranchModel* modelBranch = new XMVentBranchModel( *ui->glView3d->m_ventNet, this );
    ui->treeViewBranches->setModel( modelBranch );

    XMVentJunctionModel* modelJunction = new XMVentJunctionModel( *ui->glView3d->m_ventNet, this );
    ui->treeViewJunction->setModel( modelJunction );

    connect( ui->glView3d, SIGNAL(changed()), this, SLOT(resetSliders()) );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu menu( this );
    menu.addAction( ui->actionOpen );
    menu.exec( event->globalPos() );
}

void MainWindow::resetSliders()
{
    ui->horizontalScrollBar->setValue( int(ui->glView3d->m_camera->azimuth()) );
    ui->verticalScrollBar->setValue( int(ui->glView3d->m_camera->zenith()) );
}
