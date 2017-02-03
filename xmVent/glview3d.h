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

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>


class XMGLView3D : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT        // must include this if you use Qt signals/slots

public:
    XMGLView3D( QWidget *parent );

    class XMVentNetwork* m_ventNet;
    class XMGLCamera* m_camera;

protected:
    bool event(QEvent *event);
    bool gestureEvent( class QGestureEvent *event );
    void pinchTriggered( class QPinchGesture *gesture );
    void keyPressEvent( class QKeyEvent *event );

    // OpenGL Widget
    void initializeGL();
    void resizeGL( int width, int height );
    void paintGL();

    // OpenGL ES Shader
    QOpenGLShaderProgram mGLShaderProgram;
    int vertexLocation;
    int matrixLocation;
    int colorLocation;
    void glDrawTest();


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
