//=============================================================================================================
/**
* @file     window.cpp
* @author   Qt Project (qt3D examples)
*           Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     November, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, QtProject, Lorenz Esch and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Window class definition
*/


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "window.h"


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace DISP3DLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

Window::Window(QScreen* screen)
: QWindow(screen)
{
    setSurfaceType(QSurface::OpenGLSurface);

    resize(1024, 768);
//    QSurfaceFormat format;
//    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
//        format.setVersion(4, 3);
//        format.setProfile(QSurfaceFormat::CoreProfile);
//    }
//    format.setDepthBufferSize(24);
//    format.setSamples(4);
//    format.setStencilBufferSize(8);
//    setFormat(format);

    create();
}


//*************************************************************************************************************

Window::~Window()
{
}


//*************************************************************************************************************

void Window::keyPressEvent(QKeyEvent* e)
{
    //qDebug()<<"key press";
    switch ( e->key() )
    {
        case Qt::Key_Escape:
            QGuiApplication::quit();
            break;

        default:
            QWindow::keyPressEvent(e);
    }
}


//*************************************************************************************************************

void Window::mousePressEvent(QMouseEvent* e)
{
    //qDebug()<<"mouse press";
    QWindow::mousePressEvent(e);
}


//*************************************************************************************************************

void Window::wheelEvent(QWheelEvent* e)
{
    //qDebug()<<"mouse wheel";
    QWindow::wheelEvent(e);
}


//*************************************************************************************************************

void Window::mouseMoveEvent(QMouseEvent* e)
{
    //qDebug()<<"mouse move";
    QWindow::mouseMoveEvent(e);
}
