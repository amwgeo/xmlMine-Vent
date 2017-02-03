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


// ES Util based on https://github.com/danginsburg/opengles3-book

#define PI 3.1415926535897932384626433832795f

void esLoadIdentity( float* mat )
{
    mat[0] = 1.;
    mat[1] = 0.;
    mat[2] = 0.;
    mat[3] = 0.;

    mat[4] = 0.;
    mat[5] = 1.;
    mat[6] = 0.;
    mat[7] = 0.;

    mat[8] = 0.;
    mat[9] = 0.;
    mat[10] = 1.;
    mat[11] = 0.;

    mat[12] = 0.;
    mat[13] = 0.;
    mat[14] = 0.;
    mat[15] = 1.;
}

void esMatrixMultiply ( float *result, float *srcA, float *srcB )
{
   float temp[16];
   int i;

   for( i = 0; i < 4; i++ )
   {
      temp[4*i+0] =  ( srcA[4*i+0] * srcB[4*0+0] ) +
                     ( srcA[4*i+1] * srcB[4*1+0] ) +
                     ( srcA[4*i+2] * srcB[4*2+0] ) +
                     ( srcA[4*i+3] * srcB[4*3+0] ) ;

      temp[4*i+1] =  ( srcA[4*i+0] * srcB[4*0+1] ) +
                     ( srcA[4*i+1] * srcB[4*1+1] ) +
                     ( srcA[4*i+2] * srcB[4*2+1] ) +
                     ( srcA[4*i+3] * srcB[4*3+1] ) ;

      temp[4*i+2] =  ( srcA[4*i+0] * srcB[4*0+2] ) +
                     ( srcA[4*i+1] * srcB[4*1+2] ) +
                     ( srcA[4*i+2] * srcB[4*2+2] ) +
                     ( srcA[4*i+3] * srcB[4*3+2] ) ;

      temp[4*i+3] =  ( srcA[4*i+0] * srcB[4*0+3] ) +
                     ( srcA[4*i+1] * srcB[4*1+3] ) +
                     ( srcA[4*i+2] * srcB[4*2+3] ) +
                     ( srcA[4*i+3] * srcB[4*3+3] ) ;
   }

   memcpy ( result, &temp, 16*sizeof(float) );
}

void esFrustum ( float *mat, float left, float right, float bottom, float top, float nearZ, float farZ )
{
    float deltaX = right - left;
    float deltaY = top - bottom;
    float deltaZ = farZ - nearZ;
    float frust[16];

    if( ( nearZ <= 0.0f ) || ( farZ <= 0.0f ) ||
        ( deltaX <= 0.0f ) || ( deltaY <= 0.0f ) || ( deltaZ <= 0.0f ) )
    {
        return;
    }

    frust[0] = 2.0f * nearZ / deltaX;
    frust[1] = frust[2] = frust[3] = 0.0f;

    frust[5] = 2.0f * nearZ / deltaY;
    frust[4] = frust[6] = frust[7] = 0.0f;

    frust[8] = ( right + left ) / deltaX;
    frust[9] = ( top + bottom ) / deltaY;
    frust[10] = - ( nearZ + farZ ) / deltaZ;
    frust[11] = -1.0f;

    frust[14] = -2.0f * nearZ * farZ / deltaZ;
    frust[12] = frust[13] = frust[15] = 0.0f;

    esMatrixMultiply ( mat, frust, mat );
}

void esPerspective( float* mat, float fovy, float aspect, float nearZ, float farZ )
{
    float frustumW, frustumH;

    frustumH = tanf ( fovy / 360.0f * PI ) * nearZ;
    frustumW = frustumH * aspect;

    esFrustum( mat, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}

void esRotate( float* mat, float angle, float x, float y, float z )
{
    float sinAngle, cosAngle;
    float mag = sqrtf ( x * x + y * y + z * z );

    sinAngle = sinf ( angle * PI / 180.0f );
    cosAngle = cosf ( angle * PI / 180.0f );

    if ( mag > 0.0f )
    {
        float xx, yy, zz, xy, yz, zx, xs, ys, zs;
        float oneMinusCos;
        float rotMat[16];

        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        oneMinusCos = 1.0f - cosAngle;

        rotMat[0] = ( oneMinusCos * xx ) + cosAngle;
        rotMat[1] = ( oneMinusCos * xy ) - zs;
        rotMat[2] = ( oneMinusCos * zx ) + ys;
        rotMat[3] = 0.0F;

        rotMat[4] = ( oneMinusCos * xy ) + zs;
        rotMat[5] = ( oneMinusCos * yy ) + cosAngle;
        rotMat[6] = ( oneMinusCos * yz ) - xs;
        rotMat[7] = 0.0F;

        rotMat[8] = ( oneMinusCos * zx ) - ys;
        rotMat[9] = ( oneMinusCos * yz ) + xs;
        rotMat[10] = ( oneMinusCos * zz ) + cosAngle;
        rotMat[11] = 0.0F;

        rotMat[12] = 0.0F;
        rotMat[13] = 0.0F;
        rotMat[14] = 0.0F;
        rotMat[15] = 1.0F;

        esMatrixMultiply ( mat, rotMat, mat );
    }
}

void esTranslate( float* mat, float tx, float ty, float tz )
{
    mat[12] += mat[0] * tx + mat[4] * ty + mat[8] * tz;
    mat[13] += mat[1] * tx + mat[5] * ty + mat[9] * tz;
    mat[14] += mat[2] * tx + mat[6] * ty + mat[10] * tz;
    mat[15] += mat[3] * tx + mat[7] * ty + mat[11] * tz;
}

void XMGLCamera::glViewMatrix(float* mat, int width, int height )
{
    esLoadIdentity( mat );
    float aspect = float(width) / height;
    esPerspective( mat, 45., aspect, 0.01*m_distance, 100.*m_distance );
    esTranslate( mat, 0., 0., -m_distance );         // Stand back distance
    esRotate( mat, m_zenith, -1., 0., 0. );          // Rotate Zenuth Angle
    esRotate( mat, m_azimuth, 0., 0., 1. );          // Rotate for Azimuth
    esTranslate( mat, -m_x, -m_y, -m_z );            // Translate to centre focal point
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
