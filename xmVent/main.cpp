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

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include "mainwindow.h"

int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    a.setWindowIcon( QIcon( ":/images/xmVent.png" ) );

    QTranslator t;
    if( t.load( ":/translation/xmVent." + QLocale::system().name() ) ) {
        a.installTranslator( &t );
    }

    MainWindow w;
    w.show();

    return a.exec();
}
