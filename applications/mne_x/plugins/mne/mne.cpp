//=============================================================================================================
/**
* @file     mne.cpp
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
* @brief    Contains the implementation of the MNE class.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "mne.h"

#include "FormFiles/mnesetupwidget.h"


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtCore/QtPlugin>
#include <QtConcurrent>
#include <QDebug>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNEPlugin;
using namespace FIFFLIB;
using namespace XMEASLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

MNE::MNE()
: m_bIsRunning(false)
, m_bReceiveData(false)
, m_bProcessData(false)
, m_bFinishedClustering(false)
, m_qFileFwdSolution("./MNE-sample-data/MEG/sample/sample_audvis-meg-eeg-oct-6-fwd.fif")
, m_sAtlasDir("./MNE-sample-data/subjects/sample/label")
, m_sSurfaceDir("./MNE-sample-data/subjects/sample/surf")
, m_iNumAverages(1)
, m_iDownSample(4)
{

}


//*************************************************************************************************************

MNE::~MNE()
{
    if(this->isRunning())
        stop();
}


//*************************************************************************************************************

QSharedPointer<IPlugin> MNE::clone() const
{
    QSharedPointer<MNE> pMNEClone(new MNE());
    return pMNEClone;
}


//*************************************************************************************************************
//=============================================================================================================
// Creating required display instances and set configurations
//=============================================================================================================

void MNE::init()
{
    // Inits
    m_pFwd = MNEForwardSolution::SPtr(new MNEForwardSolution(m_qFileFwdSolution));
    m_pAnnotationSet = AnnotationSet::SPtr(new AnnotationSet(m_sAtlasDir+"/lh.aparc.a2009s.annot", m_sAtlasDir+"/rh.aparc.a2009s.annot"));
    m_pSurfaceSet = SurfaceSet::SPtr(new SurfaceSet(m_sSurfaceDir+"/lh.pial", m_sSurfaceDir+"/rh.pial"));

    // Input
    m_pRTMSAInput = PluginInputData<NewRealTimeMultiSampleArray>::create(this, "MNE RTMSA In", "MNE real-time multi sample array input data");
    connect(m_pRTMSAInput.data(), &PluginInputConnector::notify, this, &MNE::updateRTMSA, Qt::DirectConnection);
    m_inputConnectors.append(m_pRTMSAInput);

    m_pRTEInput = PluginInputData<RealTimeEvoked>::create(this, "MNE RTE In", "MNE real-time evoked input data");
    connect(m_pRTEInput.data(), &PluginInputConnector::notify, this, &MNE::updateRTE, Qt::DirectConnection);
    m_inputConnectors.append(m_pRTEInput);

    m_pRTCInput = PluginInputData<RealTimeCov>::create(this, "MNE RTC In", "MNE real-time covariance input data");
    connect(m_pRTCInput.data(), &PluginInputConnector::notify, this, &MNE::updateRTC, Qt::DirectConnection);
    m_inputConnectors.append(m_pRTCInput);

    // Output
    m_pRTSEOutput = PluginOutputData<RealTimeSourceEstimate>::create(this, "MNE Out", "MNE output data");
    m_outputConnectors.append(m_pRTSEOutput);
    m_pRTSEOutput->data()->setName(this->getName());//Provide name to auto store widget settings

    //
    // Set the fwd, annotation and surface data
    //
    m_pRTSEOutput->data()->setAnnotSet(m_pAnnotationSet);
    m_pRTSEOutput->data()->setSurfSet(m_pSurfaceSet);
    m_pRTSEOutput->data()->setFwdSolution(m_pClusteredFwd);

    // start clustering
    QFuture<void> future = QtConcurrent::run(this, &MNE::doClustering);

}


//*************************************************************************************************************

void MNE::unload()
{

}


//*************************************************************************************************************

void MNE::calcFiffInfo()
{
    QMutexLocker locker(&m_qMutex);

    if(m_qListCovChNames.size() > 0 && m_pFiffInfoInput && m_pFiffInfoForward)
    {
        qDebug() << "Fiff Infos available";

//        qDebug() << "m_qListCovChNames" << m_qListCovChNames;
//        qDebug() << "m_pFiffInfoForward->ch_names" << m_pFiffInfoForward->ch_names;
//        qDebug() << "m_pFiffInfoInput->ch_names" << m_pFiffInfoInput->ch_names;

        // Align channel names of the forward solution to the incoming averaged (currently acquired) data
        // Find out whether the forward solution depends on only MEG, EEG or both MEG and EEG channels
        QStringList forwardChannelsTypes;
        m_pFiffInfoForward->ch_names.clear();
        int counter = 0;

        for(qint32 x = 0; x < m_pFiffInfoForward->chs.size(); ++x) {
            if(forwardChannelsTypes.contains("MEG") && forwardChannelsTypes.contains("EEG"))
                break;

            if(m_pFiffInfoForward->chs[x].kind == FIFFV_MEG_CH && !forwardChannelsTypes.contains("MEG"))
                forwardChannelsTypes<<"MEG";

            if(m_pFiffInfoForward->chs[x].kind == FIFFV_EEG_CH && !forwardChannelsTypes.contains("EEG"))
                forwardChannelsTypes<<"EEG";
        }

        //If only MEG channels are used
        if(forwardChannelsTypes.contains("MEG") && !forwardChannelsTypes.contains("EEG")) {
            for(qint32 x = 0; x < m_pFiffInfoInput->chs.size(); ++x)
            {
                if(m_pFiffInfoInput->chs[x].kind == FIFFV_MEG_CH) {
                    m_pFiffInfoForward->chs[counter].ch_name = m_pFiffInfoInput->chs[x].ch_name;
                    m_pFiffInfoForward->ch_names << m_pFiffInfoInput->chs[x].ch_name;
                    counter++;
                }
            }
        }

        //If only EEG channels are used
        if(!forwardChannelsTypes.contains("MEG") && forwardChannelsTypes.contains("EEG")) {
            for(qint32 x = 0; x < m_pFiffInfoInput->chs.size(); ++x)
            {
                if(m_pFiffInfoInput->chs[x].kind == FIFFV_EEG_CH) {
                    m_pFiffInfoForward->chs[counter].ch_name = m_pFiffInfoInput->chs[x].ch_name;
                    m_pFiffInfoForward->ch_names << m_pFiffInfoInput->chs[x].ch_name;
                    counter++;
                }
            }
        }

        //If both MEG and EEG channels are used
        if(forwardChannelsTypes.contains("MEG") && forwardChannelsTypes.contains("EEG")) {
            qDebug()<<"MEG EEG fwd solution";
            for(qint32 x = 0; x < m_pFiffInfoInput->chs.size(); ++x)
            {
                if(m_pFiffInfoInput->chs[x].kind == FIFFV_MEG_CH || m_pFiffInfoInput->chs[x].kind == FIFFV_EEG_CH) {
                    m_pFiffInfoForward->chs[counter].ch_name = m_pFiffInfoInput->chs[x].ch_name;
                    m_pFiffInfoForward->ch_names << m_pFiffInfoInput->chs[x].ch_name;
                    counter++;
                }
            }
        }

        //Pick only channels which are present in all data structures (covariance, evoked and forward)
        QStringList tmp_pick_ch_names;
        foreach (const QString &ch, m_pFiffInfoForward->ch_names)
        {
            if(m_pFiffInfoInput->ch_names.contains(ch))
                tmp_pick_ch_names << ch;
        }
        m_qListPickChannels.clear();

        foreach (const QString &ch, tmp_pick_ch_names)
        {
            if(m_qListCovChNames.contains(ch))
                m_qListPickChannels << ch;
        }
        RowVectorXi sel = m_pFiffInfoInput->pick_channels(m_qListPickChannels);

        //qDebug() << "m_qListPickChannels" << m_qListPickChannels;

        m_pFiffInfo = QSharedPointer<FiffInfo>(new FiffInfo(m_pFiffInfoInput->pick_info(sel)));

        //qDebug() << "m_pFiffInfo" << m_pFiffInfo->ch_names;
    }
}


//*************************************************************************************************************

void MNE::doClustering()
{
    emit clusteringStarted();

    m_qMutex.lock();
    m_bFinishedClustering = false;
    m_pClusteredFwd = MNEForwardSolution::SPtr(new MNEForwardSolution(m_pFwd->cluster_forward_solution(*m_pAnnotationSet.data(), 40)));
    //m_pClusteredFwd = m_pFwd;
    m_pRTSEOutput->data()->setFwdSolution(m_pClusteredFwd);

    m_qMutex.unlock();

    finishedClustering();
}


//*************************************************************************************************************

void MNE::finishedClustering()
{
    m_qMutex.lock();
    m_bFinishedClustering = true;
    m_pFiffInfoForward = QSharedPointer<FiffInfoBase>(new FiffInfoBase(m_pClusteredFwd->info));
    m_qMutex.unlock();

    emit clusteringFinished();
}


//*************************************************************************************************************

bool MNE::start()
{
    //Check if the thread is already or still running. This can happen if the start button is pressed immediately after the stop button was pressed. In this case the stopping process is not finished yet but the start process is initiated.
    if(this->isRunning())
        QThread::wait();

    if(m_bFinishedClustering)
    {
        m_bIsRunning = true;
        QThread::start();
        return true;
    }
    else
        return false;
}


//*************************************************************************************************************

bool MNE::stop()
{
    m_bIsRunning = false;

    if(m_pRtInvOp->isRunning())
        m_pRtInvOp->stop();

    if(m_bProcessData) // Only clear if buffers have been initialised
    {
        m_qVecFiffEvoked.clear();
        m_qVecFiffCov.clear();
    }

    m_qListCovChNames.clear();

    // Stop filling buffers with data from the inputs
    m_bProcessData = false;

    m_bReceiveData = false;

    return true;
}


//*************************************************************************************************************

IPlugin::PluginType MNE::getType() const
{
    return _IAlgorithm;
}


//*************************************************************************************************************

QString MNE::getName() const
{
    return "MNE";
}


//*************************************************************************************************************

QWidget* MNE::setupWidget()
{
    MNESetupWidget* setupWidget = new MNESetupWidget(this);//widget is later distroyed by CentralWidget - so it has to be created everytime new

    if(!m_bFinishedClustering)
        setupWidget->setClusteringState();

    connect(this, &MNE::clusteringStarted, setupWidget, &MNESetupWidget::setClusteringState);
    connect(this, &MNE::clusteringFinished, setupWidget, &MNESetupWidget::setSetupState);

    return setupWidget;
}


//*************************************************************************************************************

void MNE::updateRTMSA(XMEASLIB::NewMeasurement::SPtr pMeasurement)
{
    QSharedPointer<NewRealTimeMultiSampleArray> pRTMSA = pMeasurement.dynamicCast<NewRealTimeMultiSampleArray>();

    if(pRTMSA && m_bReceiveData) {
        //Check if buffer initialized
        if(!m_pMatrixDataBuffer)
            m_pMatrixDataBuffer = CircularMatrixBuffer<double>::SPtr(new CircularMatrixBuffer<double>(64, pRTMSA->getNumChannels(), pRTMSA->getMultiSampleArray()[0].cols()));

        //Fiff Information of the evoked
        if(!m_pFiffInfoInput) {
            qDebug()<<"MNE::updateRTMSA - Creating m_pFiffInfoInput";
            //m_pFiffInfoInput = QSharedPointer<FiffInfo>(new FiffInfo(pRTMSA->info().data()));
            m_pFiffInfoInput = pRTMSA->info();
        }

        if(m_bProcessData)
        {
            for(qint32 i = 0; i < pRTMSA->getMultiSampleArray().size(); ++i)
            {
                MatrixXd t_mat = pRTMSA->getMultiSampleArray()[i];

                m_pMatrixDataBuffer->push(&t_mat);
            }
        }
    }
}


//*************************************************************************************************************

void MNE::updateRTC(XMEASLIB::NewMeasurement::SPtr pMeasurement)
{
    QSharedPointer<RealTimeCov> pRTC = pMeasurement.dynamicCast<RealTimeCov>();

    //MEG
    if(pRTC && m_bReceiveData)
    {
        //Fiff Information of the covariance
        if(m_qListCovChNames.size() != pRTC->getValue()->names.size())
            m_qListCovChNames = pRTC->getValue()->names;

        if(m_bProcessData)
        {
            m_qMutex.lock();
            m_qVecFiffCov.push_back(pRTC->getValue()->pick_channels(m_qListPickChannels));
            m_qMutex.unlock();
        }
    }
}



//*************************************************************************************************************

void MNE::updateRTE(XMEASLIB::NewMeasurement::SPtr pMeasurement)
{
    QSharedPointer<RealTimeEvoked> pRTE = pMeasurement.dynamicCast<RealTimeEvoked>();

    QMutexLocker locker(&m_qMutex);
    //MEG
    if(pRTE && m_bReceiveData)
    {
        //Fiff Information of the evoked
        if(!m_pFiffInfoInput)
            m_pFiffInfoInput = QSharedPointer<FiffInfo>(new FiffInfo(pRTE->getValue()->info));

        if(m_bProcessData)
            m_qVecFiffEvoked.push_back(pRTE->getValue()->pick_channels(m_qListPickChannels));
    }
}


//*************************************************************************************************************

void MNE::updateInvOp(MNEInverseOperator::SPtr p_pInvOp)
{
    m_pInvOp = p_pInvOp;

    double snr = 3.0;
    double lambda2 = 1.0 / pow(snr, 2); //ToDO estimate lambda using covariance

    QString method("dSPM"); //"MNE" | "dSPM" | "sLORETA"

    m_qMutex.lock();

    m_pMinimumNorm = MinimumNorm::SPtr(new MinimumNorm(*m_pInvOp.data(), lambda2, method));
    //
    //   Set up the inverse according to the parameters
    //
    m_pMinimumNorm->doInverseSetup(m_iNumAverages,false);
    m_qMutex.unlock();
}


//*************************************************************************************************************

void MNE::run()
{
    //
    // start receiving data
    //
    m_qMutex.lock();
    m_bReceiveData = true;
    m_qMutex.unlock();

    //
    // Read Fiff Info
    //
    while(true)
    {
        {
            QMutexLocker locker(&m_qMutex);
            if(m_pFiffInfo)
                break;
        }
        calcFiffInfo();
        msleep(10);// Wait for fiff Info
    }

    //qDebug() << "m_pClusteredFwd->info.ch_names" << m_pClusteredFwd->info.ch_names;
    //qDebug() << "m_pFiffInfo->ch_names" << m_pFiffInfo->ch_names;

    //
    // Init Real-Time inverse estimator
    //
    m_pRtInvOp = RtInvOp::SPtr(new RtInvOp(m_pFiffInfo, m_pClusteredFwd));
    connect(m_pRtInvOp.data(), &RtInvOp::invOperatorCalculated, this, &MNE::updateInvOp);
    m_pMinimumNorm.reset();

    //
    // Start the rt helpers
    //
    m_pRtInvOp->start();

    //
    // start processing data
    //
    m_bProcessData = true;

    qint32 skip_count = 0;

    while(m_bIsRunning)
    {
        m_qMutex.lock();
        qint32 t_covSize = m_qVecFiffCov.size();
        m_qMutex.unlock();
        if(t_covSize > 0)
        {
            m_qMutex.lock();
            m_pRtInvOp->appendNoiseCov(m_qVecFiffCov[0]);
            m_qVecFiffCov.pop_front();
            m_qMutex.unlock();
        }

        m_qMutex.lock();
        qint32 t_evokedSize = m_qVecFiffEvoked.size();
        m_qMutex.unlock();

        if(m_pMatrixDataBuffer)
        {
            MatrixXd rawSegment = m_pMatrixDataBuffer->pop();
            qDebug()<<"MNE::run - Processing RTMSA data";
            if(m_pMinimumNorm && ((skip_count % m_iDownSample) == 0))
            {
                float tmin = 1 / m_pFiffInfo->sfreq;
                float tstep = 1 / m_pFiffInfo->sfreq;

                m_qMutex.lock();
                MNESourceEstimate sourceEstimate = m_pMinimumNorm->calculateInverse(rawSegment, tmin, tstep);
                m_qMutex.unlock();

                m_pRTSEOutput->data()->setValue(sourceEstimate);
            }
            else
            {
                m_qMutex.lock();
                m_qVecFiffEvoked.pop_front();
                m_qMutex.unlock();
            }
            ++skip_count;
        }

        if(t_evokedSize > 0)
        {
            qDebug()<<"MNE::run - Processing RTE data";
            if(m_pMinimumNorm && ((skip_count % m_iDownSample) == 0))
            {
                m_qMutex.lock();
                FiffEvoked t_fiffEvoked = m_qVecFiffEvoked[0];
                m_qVecFiffEvoked.pop_front();
                m_qMutex.unlock();

                float tmin = ((float)t_fiffEvoked.first) / t_fiffEvoked.info.sfreq;
                float tstep = 1/t_fiffEvoked.info.sfreq;

                m_qMutex.lock();
                MNESourceEstimate sourceEstimate = m_pMinimumNorm->calculateInverse(t_fiffEvoked.data, tmin, tstep);
                m_qMutex.unlock();

                m_pRTSEOutput->data()->setValue(sourceEstimate);
            }
            else
            {
                m_qMutex.lock();
                m_qVecFiffEvoked.pop_front();
                m_qMutex.unlock();
            }
            ++skip_count;
        }
    }
}
