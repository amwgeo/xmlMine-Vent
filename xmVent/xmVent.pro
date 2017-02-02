#
#  Copyright (C) 2010 Andrew Wilson.
#  All rights reserved.
#  Contact email: amwgeo@gmail.com
#
#  This file is part of xmlMine-Vent
#
#  xmlMine-Vent is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation, either
#  version 3 of the License, or (at your option) any later version.
#
#  xmlMine-Vent is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General
#  Public License along with xmlMine-Vent.  If not, see
#  <http://www.gnu.org/licenses/>.
#

QT       += core gui widgets qml

TARGET = xmVent
TEMPLATE = app

TRANSLATIONS = translation/xmVent.fr.ts

ICON = xmVent.icns

# platform specific: Linux, MacOSX, MingW
LIBS += -L../xmVent-lib -lxmVent

INCLUDEPATH += ..   # provides access to "xmVent-lib/..."

HEADERS  += mainwindow.h glcamera.h glview3d.h branchmodel.h junctionmodel.h

SOURCES += main.cpp glcamera.cpp glview3d.cpp branchmodel.cpp junctionmodel.cpp \
        mainwindow.cpp

FORMS    += mainwindow.ui

RESOURCES += xmVent.qrc
