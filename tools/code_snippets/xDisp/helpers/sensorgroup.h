//=============================================================================================================
/**
* @file     sensorgroup.h
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     May, 2014
*
* @section  LICENSE
*
* Copyright (C) 2014, Christoph Dinh and Matti Hamalainen. All rights reserved.
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
* @brief    Declaration of the SensorGroup Class.
*
*/

#ifndef SENSORGROUP_H
#define SENSORGROUP_H

//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtXml/QDomElement>
#include <QStringList>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE XDISPLIB
//=============================================================================================================

namespace XDISPLIB
{


//=============================================================================================================
/**
* DECLARE CLASS SensorGroup
*
* @brief The SensorGroup class represents a sensor selection group
*/
class SensorGroup
{
public:
    //=========================================================================================================
    /**
    * Default constructor
    */
    SensorGroup();

    //=========================================================================================================
    /**
    * Returns the parsed sensor group
    *
    * @param [in] sensorGroupElement    the xml element which contains the sensor group
    *
    * @return the parsed sensor group
    */
    static SensorGroup parseSensorGroup(const QDomElement &sensorGroupElement);

    //=========================================================================================================
    /**
    * Returns the group name
    *
    * @return the group name
    */
    inline const QString& getGroupName() const;

    //=========================================================================================================
    /**
    * Returns the channels of the group
    *
    * @return the channels belonging to the current group
    */
    inline const QStringList& getChannelNames() const;

private:
    QString m_sGroupName;           /**< group name */
    QStringList m_qListChannels;    /**< group channel name list */
};


//*************************************************************************************************************
//=============================================================================================================
// INLINE DEFINITIONS
//=============================================================================================================

inline const QString& SensorGroup::getGroupName() const
{
    return m_sGroupName;
}


//*************************************************************************************************************

inline const QStringList& SensorGroup::getChannelNames() const
{
    return m_qListChannels;
}

} // NAMESPACE

#endif // SENSORGROUP_H
