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

#ifndef XMVENTGLCAMERA_H
#define XMVENTGLCAMERA_H

#include <QObject>

class XMGLCamera : public QObject
{
    Q_OBJECT
    Q_PROPERTY( float azimuth READ azimuth WRITE setAzimuth )
    Q_PROPERTY( float distance READ distance WRITE setDistance )
    Q_PROPERTY( float zenith READ zenith WRITE setZenith )
    Q_PROPERTY( float horizontalFOV READ horizontalFOV WRITE setHorizontalFOV )

protected:
    float m_azimuth;
    float m_distance;
    float m_zenith;
    float m_x, m_y, m_z;      // TODO:AW: make this QVector3D?
    float m_aspect;
    float m_horizontalFOV;

public:
    explicit XMGLCamera(QObject *parent = 0);

    float azimuth() const;
    float distance() const;
    float zenith() const;
    float horizontalFOV() const;

    void glViewMatrix( int width, int height ) const;

signals:
    void changed();

public slots:
    void setAzimuth( float azimuth );
    void rotateHorizontal( float angle );

    void setDistance( float distance );
    void zoom( float factor );

    void setZenith( float zenith );
    void rotateVertical( float angle );

    // get focalPoint()
    void setFocalPoint( float x, float y, float z );
    void moveFocalPoint( float dx, float dy, float dz );

    void setHorizontalFOV( float fov );
};

#endif // XMVENTGLCAMERA_H
