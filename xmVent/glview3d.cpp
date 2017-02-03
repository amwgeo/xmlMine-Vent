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
//    XMVentNetwork::fromXml( "/Users/awilson/Development/QVent/data/assignment2.xml", *mVentNet );

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
    glShadeModel( GL_SMOOTH );
    glClearColor( 1., 1., 1., 1. );
    glClearDepth( 1. );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    // Nice perspective calculations
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );


    // Setup OpenGL ES Shader Program
    /*mGLShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/basic.vert");
    mGLShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/basic.frag");
    mGLShaderProgram.link();
    mGLShaderProgram.bind();

    vertexLocation = mGLShaderProgram.attributeLocation("vertex");
    matrixLocation = mGLShaderProgram.uniformLocation("matrix");
    colorLocation = mGLShaderProgram.uniformLocation("color");//*/
}


/// setup viewport, projection etc.
void XMGLView3D::resizeGL(int width, int height)
{
    glViewport( 0, 0, width, height );
}


// Draw axis actor
/*void glDrawOrigin()
{
    glBegin( GL_LINES );
        glColor3f( 1., 0., 0. );
        glVertex3f( 0., 0., 0. );
        glVertex3f( 2., 0., 0. );

        glColor3f( 0., 1., 0. );
        glVertex3f( 0., 0., 0. );
        glVertex3f( 0., 2., 0. );

        glColor3f( 0., 0., 1. );
        glVertex3f( 0., 0., 0. );
        glVertex3f( 0., 0., 2. );
    glEnd();
}//*/


/*void glDrawModel()
{
    // Draw model lines
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_LINES );
        glVertex3f( 0.0f, 0.0f, 1.0f );
        glVertex3f(-1.0f, 1.0f, 0.0f );

        glVertex3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );

        glVertex3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( 1.0f,-1.0f, 0.0f );

        glVertex3f( 0.0f, 0.0f, 1.0f );
        glVertex3f(-1.0f,-1.0f, 0.0f );

        glVertex3f(-1.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );

        glVertex3f( 1.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f,-1.0f, 0.0f );

        glVertex3f( 1.0f,-1.0f, 0.0f );
        glVertex3f(-1.0f,-1.0f, 0.0f );

        glVertex3f(-1.0f,-1.0f, 0.0f );
        glVertex3f(-1.0f, 1.0f, 0.0f );
    glEnd();

    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 1., 1. );

    // Draw model solids
    glColor3f( 0.5f, 0.5f, 1.0f );
    glBegin( GL_QUADS );
        glVertex3f(-1.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f,-1.0f, 0.0f );
        glVertex3f(-1.0f,-1.0f, 0.0f );
    glEnd();

    glBegin( GL_TRIANGLES );
        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 0.0f, 0.0f, 1.0f );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f(-1.0f, 1.0f, 0.0f );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );

        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 0.0f, 0.0f, 1.0f );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( 1.0f,-1.0f, 0.0f );

        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 0.0f, 0.0f, 1.0f );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( 1.0f,-1.0f, 0.0f );
        glColor3f( 0.0f, 1.0f, 1.0f );
        glVertex3f(-1.0f,-1.0f, 0.0f );

        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 0.0f, 0.0f, 1.0f );
        glColor3f( 0.0f, 1.0f, 1.0f );
        glVertex3f(-1.0f,-1.0f, 0.0f );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f(-1.0f, 1.0f, 0.0f );
    glEnd();

    glPopAttrib();
}//*/


void glVertex( const XMVentJunction* junction )
{
    const QVector3D& point( junction->point() );
    glVertex3f( point.x(), point.y(), point.z() );
}


void glDrawNetworkModel( const XMVentNetwork* net, QOpenGLWidget* /*widget*/ )
{

    // Draw Nodes
    glPointSize( 5. );
    QVector<XMVentJunction*>::const_iterator itJunct;
    glColor3f( 0., 0., 1. );
    glBegin( GL_POINTS );
    for( itJunct = net->m_junction.begin(); itJunct != net->m_junction.end(); itJunct++ ) {
        glVertex( *itJunct );
    }
    glEnd();

    // Draw Branches
    glColor3f( 0., 1., 0. );
    glLineStipple( 5, 0xAAAA );
    QVector<XMVentBranch*>::const_iterator itBranch;
    for( itBranch = net->m_branch.begin(); itBranch != net->m_branch.end(); itBranch++ ) {
        XMVentJunction* from = net->m_junction[ (*itBranch)->fromId() ];
        XMVentJunction* to = net->m_junction[ (*itBranch)->toId() ];
        bool surface = from->isSurface() && to->isSurface();
        if( surface ) {
            glEnable( GL_LINE_STIPPLE );
        }
        glBegin( GL_LINES );
            glVertex( from );
            glVertex( to );
        glEnd();
        if(surface) {
            glDisable( GL_LINE_STIPPLE );
        }
    }

    /*glDisable( GL_DEPTH_TEST );
    // Draw Node Lables
    glColor3f( 0., 0., 1. );
    for( itJunct = net->junction.begin(); itJunct != net->junction.end(); itJunct++ ) {
        XMVentJunction *j1 = *itJunct;
        widget->renderText( j1->point.x(), j1->point.y(), j1->point.z(), j1->id );
    }
    // Draw Branch Lables
    glColor3f( 0., 1., 0. );
    for( itBranch = net->branch.begin(); itBranch != net->branch.end(); itBranch++ ) {
        QVector3D point = ( net->junction[ (*itBranch)->from ]->point
                          + net->junction[ (*itBranch)->to ]->point ) / 2.;
        widget->renderText( point.x(), point.y(), point.z(), (*itBranch)->id );
    }
    glEnable( GL_DEPTH_TEST );//*/

}

/// draw the scene
void XMGLView3D::paintGL()
{
    //TODO?: beginNativePainting();

    // Setup projection matrix
    glMatrixMode( GL_PROJECTION );
    m_camera->glViewMatrix( width(), height() );

    // Clear the screen reset for next drawing
    glMatrixMode( GL_MODELVIEW );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glLoadIdentity();

//    glDrawOrigin();
//    glDrawModel();
    glDrawNetworkModel( m_ventNet, this );

    //TODO?: endNativePainting();
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
