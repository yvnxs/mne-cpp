//=============================================================================================================
/**
* @file     realtimemultisamplearray_new.cpp
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     February, 2013
*
* @section  LICENSE
*
* Copyright (C) 2013, Christoph Dinh and Matti Hamalainen. All rights reserved.
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
* @brief    Contains the implementation of the RealTimeMultiSampleArrayNew class.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "realtimemultisamplearray_new.h"


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QDebug>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace XMEASLIB;
//using namespace IOBuffer;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

RealTimeMultiSampleArrayNew::RealTimeMultiSampleArrayNew()
: MltChnMeasurement()
, m_dSamplingRate(0)
, m_ucMultiArraySize(10)
{

}


//*************************************************************************************************************

RealTimeMultiSampleArrayNew::~RealTimeMultiSampleArrayNew()
{

}


//*************************************************************************************************************

void RealTimeMultiSampleArrayNew::init(unsigned int uiNumChannels)
{
    m_qListChInfo.clear();

    for(quint32 i = 0; i < uiNumChannels; ++i)
    {
        RealTimeSampleArrayChInfo initChInfo;
        m_qListChInfo.append(initChInfo);
    }
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayNew::initFromFiffInfo(FiffInfo::SPtr &p_pFiffInfo)
{
    m_qListChInfo.clear();

    bool t_bIsBabyMEG = false;

    if(p_pFiffInfo->acq_pars == "BabyMEG")
        t_bIsBabyMEG = true;

    for(qint32 i = 0; i < p_pFiffInfo->nchan; ++i)
    {
        RealTimeSampleArrayChInfo initChInfo;
        initChInfo.setChannelName(p_pFiffInfo->chs[i].ch_name);

        initChInfo.setUnit(p_pFiffInfo->chs[i].unit);

        //Treat stimulus channels different
        if(p_pFiffInfo->chs[i].kind == FIFFV_STIM_CH)
        {
            initChInfo.setMinValue(0);
            initChInfo.setMaxValue(1.0e6);
        }
//        else
//        {
////            qDebug() << "kind" << p_pFiffInfo->chs[i].kind << "unit" << p_pFiffInfo->chs[i].unit;

//            //Unit
//            switch(p_pFiffInfo->chs[i].unit)
//            {
//                case 101:
//                    initChInfo.setUnit("Hz");
//                    break;
//                case 102:
//                    initChInfo.setUnit("N");
//                    break;
//                case 103:
//                    initChInfo.setUnit("Pa");
//                    break;
//                case 104:
//                    initChInfo.setUnit("J");
//                    break;
//                case 105:
//                    initChInfo.setUnit("W");
//                    break;
//                case 106:
//                    initChInfo.setUnit("C");
//                    break;
//                case 107:
//                    initChInfo.setUnit("V");
//                    initChInfo.setMinValue(0);
//                    initChInfo.setMaxValue(1.0e-3);
//                    break;
//                case 108:
//                    initChInfo.setUnit("F");
//                    break;
//                case 109:
//                    initChInfo.setUnit("Ohm");
//                    break;
//                case 110:
//                    initChInfo.setUnit("MHO");
//                    break;
//                case 111:
//                    initChInfo.setUnit("Wb");
//                    break;
//                case 112:
//                    initChInfo.setUnit("T");
//                    if(t_bIsBabyMEG)
//                    {
//                        initChInfo.setMinValue(-1.0e-4);
//                        initChInfo.setMaxValue(1.0e-4);
//                    }
//                    else
//                    {
//                        initChInfo.setMinValue(-1.0e-10);
//                        initChInfo.setMaxValue(1.0e-10);
//                    }
//                    break;
//                case 113:
//                    initChInfo.setUnit("H");
//                    break;
//                case 114:
//                    initChInfo.setUnit("Cel");
//                    break;
//                case 115:
//                    initChInfo.setUnit("Lm");
//                    break;
//                case 116:
//                    initChInfo.setUnit("Lx");
//                    break;
//                case 201:
//                    initChInfo.setUnit("T/m");
//                    if(t_bIsBabyMEG)
//                    {
//                        initChInfo.setMinValue(-1.0e-4);
//                        initChInfo.setMaxValue(1.0e-4);
//                    }
//                    else
//                    {
//                        initChInfo.setMinValue(-1.0e-10);
//                        initChInfo.setMaxValue(1.0e-10);
//                    }
//                    break;
//                case 202:
//                    initChInfo.setUnit("Am");
//                    break;
//                default:
//                    initChInfo.setUnit("");
//            }
//        }

        // set channel Kind
        initChInfo.setKind(p_pFiffInfo->chs[i].kind);

        m_qListChInfo.append(initChInfo);
    }

    //Sampling rate
    m_dSamplingRate = p_pFiffInfo->sfreq;


    m_pFiffInfo_orig = p_pFiffInfo;
}


//*************************************************************************************************************

VectorXd RealTimeMultiSampleArrayNew::getValue() const
{
    return m_vecValue;
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayNew::setValue(VectorXd v)
{
    //check vector size
    if(v.size() != m_qListChInfo.size())
        qCritical() << "Error Occured in RealTimeMultiSampleArrayNew::setVector: Vector size does not matche the number of channels! ";

    //Check if maximum exceeded //ToDo speed this up
    for(qint32 i = 0; i < v.size(); ++i)
    {
        if(v[i] < m_qListChInfo[i].getMinValue()) v[i] = m_qListChInfo[i].getMinValue();
        else if(v[i] > m_qListChInfo[i].getMaxValue()) v[i] = m_qListChInfo[i].getMaxValue();
    }

    //Store
    m_vecValue = v;
    m_matSamples.push_back(m_vecValue);
    if(m_matSamples.size() >= m_ucMultiArraySize && notifyEnabled)
    {
        notify();
        m_matSamples.clear();
    }
}
