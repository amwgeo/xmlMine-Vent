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

#ifndef XMVENTGLVIEW3D_H
#define XMVENTGLVIEW3D_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    #include <QOpenGLExtraFunctions>
#else
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
#endif


#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QMatrix>
#include <QOpenGLBuffer>

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    class XMGLView3D : public QOpenGLWidget, protected QOpenGLExtraFunctions
#else
    #include <QOpenGLFunctions>
    class XMGLView3D : public QOpenGLWidget, protected QOpenGLFunctions
#endif
{
    Q_OBJECT        // must include this if you use Qt signals/slots

public:
    XMGLView3D( QWidget *parent );

    class XMVentNetwork* m_ventNet;
    class XMGLCamera* m_camera;

    void glDrawNetworkModel();
    void glDrawNetworkNodes( const QVector<QVector3D> &vertexData );

protected:
    bool event(QEvent *event);
    void selectEvent(const QPoint &pos);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    bool gestureEvent( class QGestureEvent *event );
    void pinchTriggered( class QPinchGesture *gesture );
    void keyPressEvent( class QKeyEvent *event );

    // OpenGL Widget
    void initializeGL();
    void resizeGL( int width, int height );
    void paintGL();

    // OpenGL ES Shader
    QOpenGLShaderProgram mShaderBasic;
    QOpenGLShaderProgram mShaderNodes;

    // Mouse Event Last Posision
    QPointF lastPos;

    // OpenGL
    QMatrix4x4 m_MV;
    QMatrix4x4 m_MVP;
    QMatrix3x3 m_Norm;
    QOpenGLBuffer m_vboNodes;
    QOpenGLBuffer m_iboNodes;

signals:
    void changed();

public slots:
    void open();
    void setVerticalAngle( int value );
    void setHorizontalAngle( int value );

protected slots:
    void dependentChanged();
};

#endif // XMVENTGLVIEW3D_H
