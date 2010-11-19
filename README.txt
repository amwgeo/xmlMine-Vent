Copyright (C) 2010 Andrew Wilson.
All rights reserved.
Contact email: amwgeo@gmail.com

This file is part of xmlMine-Vent

xmlMine-Vent is free software: you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

xmlMine-Vent is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General
Public License along with xmlMine-Vent.  If not, see
<http://www.gnu.org/licenses/>.



------------------------- Introduction -------------------------
xmlMine-Vent can solve mine ventilation problems using a Hardy-Cross
formulated incompressible solver.


----------------------- Build from source ----------------------
1. Download and install LGPL version of Qt SDK (qt.nokia.com/downloads)
2. Unpack the source files or get the latest version using git.
3. cd [xmVent source directory]
4. qmake
5. make

NOTE: some platforms might require slight variations to this process.

Mac OSX Extra Steps:
(from inside the xmVent build directory)
mkdir -p xmVent.app/Contents/Frameworks
cp [xmVent-lib-build]/libxmvent.1.dylib xmVent.app/Contents/Frameworks/
install_name_tool -id @executable_path/../Frameworks/libxmvent.1.dylib xmVent.app/Contents/Frameworks/libxmvent.1.dylib
install_name_tool -change libxmvent.1.dylib @executable_path/../Frameworks/libxmvent.1.dylib xmVent.app/Contents/MacOS/xmVent

---------------------- File organization ----------------------
README.txt	- this file
LICENCE.*.txt	- GPL / LGPL Licences
xmVent/		- OpenGL based User Interface
xmVent-lib/	- Ventilation solver and network element classes
data/		- example XML networks

