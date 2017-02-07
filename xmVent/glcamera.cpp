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

#include "glcamera.h"

#include <cmath>
using namespace std;


XMGLCamera::XMGLCamera(QObject *parent) :
    QObject(parent)
{
    m_azimuth = 30.;
    m_distance = 10.;
    m_zenith = 80.;
    m_x = 0.;
    m_y = 0.;
    m_z = 0.;
    m_aspect = 4. / 3.;
    m_horizontalFOV = 45.;
}


float XMGLCamera::azimuth() const
{
    return m_azimuth;
}


void XMGLCamera::setAzimuth( float azimuth )
{
    m_azimuth = fmod( azimuth, 360.f );
    if( m_azimuth < 0.f ) {
        m_azimuth += 360.f;
    }
    emit changed();
}


void XMGLCamera::rotateHorizontal( float angle )
{
    setAzimuth( m_azimuth + angle );
}


float XMGLCamera::distance() const
{
    return m_distance;
}


void XMGLCamera::setDistance( float distance )
{
    m_distance = distance;
    emit changed();
}


void XMGLCamera::zoom( float factor )
{
    m_distance *= factor;
    emit changed();
}


float XMGLCamera::zenith() const
{
    return m_zenith;
}


void XMGLCamera::setZenith( float zenith )
{
    // Clamp to 0 .. 180
    if( zenith < 0.f ) {
        m_zenith = 0.f;
    } else if( zenith > 180.f ) {
        m_zenith = 180.f;
    } else {
        m_zenith = zenith;
    }
    emit changed();
}


void XMGLCamera::rotateVertical( float angle )
{
    setZenith( m_zenith + angle );
}


void XMGLCamera::setFocalPoint( float x, float y, float z )
{
    m_x = x;
    m_y = y;
    m_z = z;
    emit changed();
}


void XMGLCamera::moveFocalPoint( float dx, float dy, float dz )
{
    m_x += dx;
    m_y += dy;
    m_z += dz;
    emit changed();
}


float XMGLCamera::horizontalFOV() const
{
    return m_horizontalFOV;
}


void XMGLCamera::setHorizontalFOV( float fov )
{
    m_horizontalFOV = fov;
}

QMatrix4x4 XMGLCamera::glProjMatrix( int width, int height ) const
{
    QMatrix4x4 mat;
    float aspect = float(width) / height;
    mat.perspective( 45., aspect, 0.01*m_distance, 100.*m_distance );

    return mat;
}

QMatrix4x4 XMGLCamera::glViewMatrix() const
{
    QMatrix4x4 mat;
    mat.translate( 0., 0., -m_distance );           // Stand back distance
    mat.rotate( m_zenith, -1., 0., 0. );            // Rotate Zenuth Angle
    mat.rotate( m_azimuth, 0., 0., 1. );            // Rotate for Azimuth
    mat.translate( -m_x, -m_y, -m_z );              // Translate to centre focal point

    return mat;
}



// Old OpenGL
/*
void XMGLCamera::glViewMatrix( int width, int height ) const
{
    glLoadIdentity();
    float aspect = float(width) / height;
    gluPerspective( 45., aspect, 0.01*m_distance, 100.*m_distance );
    glTranslatef( 0., 0., -m_distance );         // Stand back distance
    glRotatef( m_zenith, -1., 0., 0. );          // Rotate Zenuth Angle
    glRotatef( m_azimuth, 0., 0., 1. );          // Rotate for Azimuth
    glTranslatef( -m_x, -m_y, -m_z );            // Translate to centre focal point

}//*/
