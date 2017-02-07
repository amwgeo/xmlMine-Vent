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
#include <QPainter>

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


void setupShaderProgramFromFiles(
        QOpenGLShaderProgram &shader,
        const QString &fileVert,
        const QString &fileFrag,
        const char *errorName )
{
    // Setup OpenGL ES Shader Program
    if( !shader.addShaderFromSourceFile(QOpenGLShader::Vertex, fileVert) ) {
        qFatal("Error compiling vertex shader (%s):\n%s", errorName, shader.log().toStdString().c_str());
    }
    if( !shader.addShaderFromSourceFile(QOpenGLShader::Fragment, fileFrag) ) {
        qFatal("Error compiling fragment shader (%s):\n%s:", errorName, shader.log().toStdString().c_str());
    }
    if( !shader.link() ) {
        qFatal("Error linking shaders (%s):\n%s", errorName, shader.log().toStdString().c_str());
    }
    if( !shader.bind() ) {
        qFatal("Error binding shader (%s).", errorName);
    }
}


/// Set up the rendering context, define display lists etc.
void XMGLView3D::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor( 1., 1., 1., 1. );

    qDebug() << "GL Version: " << QString::fromLocal8Bit((const char*)glGetString(GL_VERSION));
    qDebug() << "GLSL Version: " << QString::fromLocal8Bit((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    qDebug() << "GL Extensions: "  << QString::fromLocal8Bit((const char*)glGetString(GL_EXTENSIONS));

    setupShaderProgramFromFiles( mShaderBasic, ":/shaders/basic.vert", ":/shaders/basic.frag", "mShaderBasic");
    setupShaderProgramFromFiles( mShaderNodes, ":/shaders/nodes.vert", ":/shaders/nodes.frag","mShaderNodes" );
}


/// setup viewport, projection etc.
void XMGLView3D::resizeGL(int width, int height)
{
    // TODO: Is this or otherwise needed in GLES?
    glViewport( 0, 0, width, height );
}


void XMGLView3D::glDrawNetworkNodes( const QVector<QVector3D> &vertexData )
{
    // draw blue dots (old way)
    //mShaderBasic.setUniformValue("color", QColor("darkblue"));
    //mShaderBasic.setUniformValue("pointSize", float(5.));
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glDrawArrays( GL_POINTS, 0, m_ventNet->m_junction.size() );

    // draw ichosahedrons (new way)
    const float a = 0.525731112119134;
    const float b = 0.850650808352040;
    const static QVector<QVector3D> ichosaVert( {
                        QVector3D(0,a,b),   QVector3D(0,a,-b), QVector3D(0,-a,b),
                        QVector3D(0,-a,-b), QVector3D(a,b,0),  QVector3D(a,-b,0),
                        QVector3D(-a,b,0),  QVector3D(-a,-b,0),QVector3D(b,0,a),
                        QVector3D(-b,0,a),  QVector3D(b,0,-a), QVector3D(-b,0,-a) } );

    const static QVector<GLuint> triElements( {
                        0,2,8,  0,8,4,  0,4,6,  0,6,9,  0,9,2,
                        1,10,3, 1,3,11, 1,11,6, 1,6,4,  1,4,10,
                        2,9,7,  2,7,5,  2,5,8,  3,10,5, 3,5,7,
                        3,7,11, 4,8,10, 5,10,8, 6,11,9, 7,9,11 } );

    mShaderNodes.bind();
    mShaderNodes.setUniformValue("u_matMVP", m_MVP);
    mShaderNodes.setUniformValue("u_matMV", m_MV);
    mShaderNodes.setUniformValue("u_matNorm", m_Norm);
    mShaderNodes.enableAttributeArray("a_vertex");
    mShaderNodes.setAttributeArray("a_vertex", ichosaVert.constData());
    mShaderNodes.enableAttributeArray("a_normal");
    mShaderNodes.setAttributeArray("a_normal", ichosaVert.constData());
    mShaderNodes.setUniformValue("u_size",GLfloat(5));
    mShaderNodes.setUniformValue("u_coluorMaterial", QColor("darkred") );
    mShaderNodes.setUniformValueArray("a_offset",vertexData.constData(), vertexData.size() );

    // Draw the really inefficient way ...
    for( int i=0; i<vertexData.count(); i++ ) {
        QMatrix4x4 MVP = m_MVP;
        MVP.translate( vertexData[i] );
        mShaderNodes.setUniformValue("u_matMVP", MVP);
        QMatrix4x4 MV = m_MV;
        MV.translate( vertexData[i] );
        mShaderNodes.setUniformValue("u_matMV", MV);
        mShaderNodes.setUniformValue("u_matNorm", MV.normalMatrix());
        glDrawElements( GL_TRIANGLES, triElements.size(), GL_UNSIGNED_INT, triElements.constData() );
    }

    // TODO: get instance working ...
    //glDrawElementsInstanced(
    //            GL_TRIANGLES,
    //            triElements.size(),
    //            GL_UNSIGNED_INT,
    //            triElements.constData(),
    //            vertexData.size() );

    mShaderNodes.disableAttributeArray("a_vertex");
    mShaderNodes.disableAttributeArray("a_normal");
}


void XMGLView3D::glDrawNetworkModel()
{
    if(m_ventNet == 0 || m_ventNet->m_junction.size() == 0) return;

    mShaderBasic.enableAttributeArray("vertex");

    //TODO: figure out direct access to data stored in existing junction vector ...
    //int stride = 0;
    //if( m_ventNet->m_junction.size() > 1 ) {
    //    //stride = &(m_ventNet->m_junction[1]->m_point) - &(m_ventNet->m_junction[0]->m_point);
    //    stride = sizeof(XMVentJunction);
    //}
    //QVector3D *data2 = &(m_ventNet->m_junction[0]->m_point);
    //QVector3D *data = (QVector3D*)(m_ventNet->m_junction.data() + offsetof(class XMVentJunction, m_point));
    //mShaderBasic.setAttributeArray("vertex", data, stride);

    // copy out vertex data into convienient for purpose vector ...
    QVector<QVector3D> vertexData;
    for(int i=0; i< m_ventNet->m_junction.size(); i++ ) {
        vertexData.append(m_ventNet->m_junction[i]->point());
    }

    // copy out element data from branches
    QVector<GLuint> elements;
    for(int j=0; j<m_ventNet->m_branch.size(); j++) {
        elements.append(m_ventNet->m_branch[j]->fromId());
        elements.append(m_ventNet->m_branch[j]->toId());
    }

    // draw green lines
    mShaderBasic.bind();
    mShaderBasic.setAttributeArray("vertex", vertexData.constData());
    mShaderBasic.setUniformValue("matrix", m_MVP);
    mShaderBasic.setUniformValue("color", QColor("lime"));
    glDrawElements(GL_LINES, elements.size(), GL_UNSIGNED_INT, elements.constData());
    mShaderBasic.disableAttributeArray("vertex");

    // draw the nodes
    glDrawNetworkNodes( vertexData );

}


/// draw the scene
void XMGLView3D::paintGL()
{
    m_MV = m_camera->glViewMatrix();
    m_MVP = m_camera->glProjMatrix( width(), height() ) * m_MV;
    m_Norm = m_MV.normalMatrix();

    QPainter painter(this);

    // Draw gradient background
    QLinearGradient linearGrad( QPointF(0, 0), QPointF(0, height()) );
    linearGrad.setColorAt( 0, QColor("slategray")  );
    linearGrad.setColorAt( 1, QColor("skyblue") );
    painter.setBrush( QBrush(linearGrad) );
    painter.drawRect( rect() );

    // Draw 3D model
    painter.beginNativePainting();

    glClear( GL_DEPTH_BUFFER_BIT );
    glEnable( GL_CULL_FACE );

    glDrawNetworkModel();

    glDisable( GL_CULL_FACE );

    painter.endNativePainting();

    // Draw Junction Id
    painter.setPen( Qt::black );
    painter.setFont( QFont() );
    QMatrix4x4 matViewport;
    matViewport.viewport( rect() );
    matViewport.scale(1., -1., 1.);

    // network node id
    for(int i=0; i< m_ventNet->m_junction.size(); i++ ) {
        XMVentJunction *junction = m_ventNet->m_junction[i];
        QVector3D p = matViewport * m_MVP * junction->point();
        painter.drawText( p.x(), p.y(), junction->id() );
    }

    painter.end();
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
