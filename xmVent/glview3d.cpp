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

#include "glview3d.h"

#include <QGestureEvent>
#include <QPinchGesture>
#include <QDebug>
#include <QFileDialog>

#include "glcamera.h"
#include "xmVent-lib/network.h"
#include "xmVent-lib/solvehc.h"
#include "xmVent-lib/branch.h"
#include "xmVent-lib/junction.h"


XMGLView3D::XMGLView3D( QWidget *parent ):
        QOpenGLWidget( parent )
{
    // TODO: memory leak or delete from parent?
    m_camera = new XMGLCamera( this );
    connect( m_camera, SIGNAL(changed()), this, SLOT(update()) );

    m_ventNet = new XMVentNetwork( this );
//    XMVentNetwork::fromXml( "data/assignment2.xml", *mVentNet );

    grabGesture(Qt::PinchGesture);

    setFocusPolicy( Qt::WheelFocus );   // TODO: is this a good choice?

    connect( m_camera, SIGNAL(changed()), this, SLOT(dependentChanged()) );
}


bool XMGLView3D::event(QEvent *event)
{
    switch( event->type() ) {
    case QEvent::Gesture:
        return gestureEvent( static_cast<QGestureEvent*>(event) );

    default:
        break;
    }

    // Handle the un-handled with base class implemtation
    return QOpenGLWidget::event(event);
}


bool XMGLView3D::gestureEvent(QGestureEvent *event)
{
    if( QGesture *pinch = event->gesture(Qt::PinchGesture) )
        pinchTriggered( static_cast<QPinchGesture *>(pinch) );

    return true;
}


void XMGLView3D::pinchTriggered(QPinchGesture *gesture)
{
    if( gesture->changeFlags() & QPinchGesture::RotationAngleChanged ) {
        m_camera->rotateHorizontal( gesture->lastRotationAngle() - gesture->rotationAngle() );
    }

    if( gesture->changeFlags() & QPinchGesture::ScaleFactorChanged ) {
       m_camera->zoom( 1. / gesture->scaleFactor() );
    }
}


void XMGLView3D::keyPressEvent( class QKeyEvent *event )
{
    switch( event->key() ) {
    case Qt::Key_Z:
        m_camera->rotateVertical( 5. );
        break;

    case Qt::Key_X:
        m_camera->rotateVertical( -5. );
        break;

    default:
        QOpenGLWidget::keyPressEvent( event );
        break;
    }
}


/// Set up the rendering context, define display lists etc.
void XMGLView3D::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor( 1., 1., 1., 1. );

    // Setup OpenGL ES Shader Program
    if( !mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/basic.vert") ) {
        qFatal("Error compiling vertex shader:\n%s", mShaderProgram.log().toStdString().c_str());
    }
    if( !mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/basic.frag") ) {
        qFatal("Error compiling fragment shader:\n%s:", mShaderProgram.log().toStdString().c_str());
    }
    if( !mShaderProgram.link() ) {
        qFatal("Error linking shaders:\n%s", mShaderProgram.log().toStdString().c_str());
    }
    if( !mShaderProgram.bind() ) {
        qFatal("Error binding shader.");
    }

}


/// setup viewport, projection etc.
void XMGLView3D::resizeGL(int width, int height)
{
    // TODO: Is this or otherwise needed in GLES?
    glViewport( 0, 0, width, height );
}


void XMGLView3D::glDrawNetworkModel()
{
    if(m_ventNet == 0 || m_ventNet->m_junction.size() == 0) return;

    mShaderProgram.enableAttributeArray("vertex");

    //TODO: figure out direct access to data stored in existing junction vector ...
    //int stride = 0;
    //if( m_ventNet->m_junction.size() > 1 ) {
    //    //stride = &(m_ventNet->m_junction[1]->m_point) - &(m_ventNet->m_junction[0]->m_point);
    //    stride = sizeof(XMVentJunction);
    //}
    //QVector3D *data2 = &(m_ventNet->m_junction[0]->m_point);
    //QVector3D *data = (QVector3D*)(m_ventNet->m_junction.data() + offsetof(class XMVentJunction, m_point));
    //mShaderProgram.setAttributeArray("vertex", data, stride);

    // copy out vertex data into convienient for purpose vector ...
    QVector<QVector3D> vertexData;
    for(int i=0; i< m_ventNet->m_junction.size(); i++ ) {
        vertexData.append(m_ventNet->m_junction[i]->point());
    }
    mShaderProgram.setAttributeArray("vertex", vertexData.constData());

    // copy out element data from branches
    QVector<GLuint> elements;
    for(int j=0; j<m_ventNet->m_branch.size(); j++) {
        elements.append(m_ventNet->m_branch[j]->fromId());
        elements.append(m_ventNet->m_branch[j]->toId());
    }

    // draw green lines
    mShaderProgram.setUniformValue("color", QColor(0,255,0,255));
    glDrawElements(GL_LINES, elements.size(), GL_UNSIGNED_INT, elements.constData());

    // draw blue dots
    mShaderProgram.setUniformValue("color", QColor(0,0,255,255));
//    glPointSize( 5.f );       // TODO GLES: set gl_PointSize in vertex shader
    glDrawArrays( GL_POINTS, 0, m_ventNet->m_junction.size() );

    // cleanup and make safe until next time
    mShaderProgram.disableAttributeArray("vertex");

}


/// draw the scene
void XMGLView3D::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_MVP = m_camera->glViewMatrix( width(), height() );

    mShaderProgram.setUniformValue("matrix", m_MVP);

    glDrawNetworkModel();
}


void XMGLView3D::open()
{
    // TODO: check for errors
    QString fileName = QFileDialog::getOpenFileName( this,
        tr("Open Ventilation Network File"), QString(), tr("Ventilation Network XML (*.xml)") );

    if( fileName.isEmpty() ) {
        return;
    }
    qDebug() << "Opening " << fileName;

    m_ventNet->fromXml( fileName );
    float x0, y0, z0, x1, y1, z1;
    m_ventNet->getLimits( x0, y0, z0, x1, y1, z1 );
    m_camera->setFocalPoint( (x0 + x1) / 2., (y0 + y1) / 2., (z0 + z1) / 2. );
    x1 -= x0;
    y1 -= y0;
    z1 -= z0;
    float distance = x1 > y1 ? x1 : y1;
    if( z1 > distance ) distance = z1;
    m_camera->setDistance( distance );

//    ui->glView3d->updateGL();   // TODO: make this a signal/slot?
    // TODO: refresh QTreeViews ...

    // Hardy Cross Solver
    m_ventNet->m_solver.solve();
    qDebug() << "from,to,flow";
    for( int i = 0; i < m_ventNet->m_branch.count(); i++ ) {
        //qDebug() << "branch" << mVentNet->branch[i]->id() << "flow" << mVentNet->solver.flowList[i];
        XMVentBranch* branch = m_ventNet->m_branch[i];
        qDebug() << m_ventNet->m_junction[ branch->fromId() ]->id() << "," << m_ventNet->m_junction[ branch->toId() ]->id() << "," << m_ventNet->m_solver.m_flowList[i];
    }

    update();
}

void XMGLView3D::setVerticalAngle( int value )
{
    m_camera->setZenith( float(value) );
}


void XMGLView3D::setHorizontalAngle( int value )
{
    m_camera->setAzimuth( float(value) );
}

void XMGLView3D::dependentChanged()
{
    emit changed();
}
