//=============================================================================================================
/**
* @file     commandthread.h
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     July, 2012
*
* @section  LICENSE
*
* Copyright (C) 2012, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of the Massachusetts General Hospital nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MASSACHUSETTS GENERAL HOSPITAL BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Contains the implementation of the CommandThread Class.
*
*/

#ifndef COMMANDTHREAD_H
#define COMMANDTHREAD_H

//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QThread>
#include <QTcpSocket>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE MSERVER
//=============================================================================================================

namespace MSERVER
{

class CommandThread : public QThread
{
    Q_OBJECT

public:
    CommandThread(int socketDescriptor, QObject *parent);

    ~CommandThread();

//    QByteArray availableCommands() const;

    void run();

signals:
    void error(QTcpSocket::SocketError socketError);

    void requestMeasInfo(qint32 p_iClientId);
    void requestStartMeas(qint32 p_iClientId);
    void requestStopMeas(qint32 p_iClientId);
    void requestStopConnector();
//    void requestSetBufferSize(quint32 p_uiBuffSize);

private:
    bool parseCommand(QTcpSocket& p_qTcpSocket, QString& p_sCommand);

//    QByteArray parseToId(QString& p_sRawId, qint32& p_iParsedId);

    int socketDescriptor;

    bool m_bIsRunning;

};


} // NAMESPACE

#endif //COMMANDTHREAD_H
