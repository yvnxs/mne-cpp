//=============================================================================================================
/**
* @file     realtimesamplearraywidget.cpp
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
* @brief    Implementation of the RealTimeMultiSampleArrayWidget Class.
*
*/

//ToDo Paint to render area

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "realtimemultisamplearraywidget.h"


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace XDISPLIB;
using namespace XMEASLIB;


//=============================================================================================================
/**
* Tool enumeration.
*/
enum Tool
{
    Freeze     = 0,     /**< Freezing tool. */
    Annotation = 1      /**< Annotation tool. */
};


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

RealTimeMultiSampleArrayWidget::RealTimeMultiSampleArrayWidget(QSharedPointer<NewRealTimeMultiSampleArray> pRTMSA, QSharedPointer<QTime> &pTime, QWidget* parent)
: NewMeasurementWidget(parent)
, m_pRTMSAModel(NULL)
, m_pRTMSADelegate(NULL)
, m_pTableView(NULL)
, m_fDefaultSectionSize(80.0f)
, m_fZoomFactor(1.0f)
, m_pRTMSA(pRTMSA)
, m_bInitialized(false)
, m_iT(10)
, m_fSamplingRate(1024)
, m_bHideBadChannels(false)
, m_iMaxFilterTapSize(0)
, m_pQuickControlWidget(Q_NULLPTR)
, m_pChInfoModel(Q_NULLPTR)
, m_pSelectionManagerWindow(Q_NULLPTR)
, m_pFilterWindow(Q_NULLPTR)
{
    Q_UNUSED(pTime)

    m_pActionSelectSensors = new QAction(QIcon(":/images/selectSensors.png"), tr("Shows the region selection widget (F9)"),this);
    m_pActionSelectSensors->setShortcut(tr("F9"));
    m_pActionSelectSensors->setToolTip(tr("Shows the region selection widget (F9)"));
    connect(m_pActionSelectSensors, &QAction::triggered,
            this, &RealTimeMultiSampleArrayWidget::showSensorSelectionWidget);
    addDisplayAction(m_pActionSelectSensors);
    m_pActionSelectSensors->setVisible(true);

    m_pActionFiltering = new QAction(QIcon(":/images/showFilterWindow.png"), tr("Shows the filter window (F11)"),this);
    m_pActionFiltering->setShortcut(tr("F11"));
    m_pActionFiltering->setStatusTip(tr("Shows the filter window (F11)"));
    connect(m_pActionFiltering, &QAction::triggered,
            this, &RealTimeMultiSampleArrayWidget::showFilterWidget);
    addDisplayAction(m_pActionFiltering);
    m_pActionFiltering->setVisible(false);

    m_pActionHideBad = new QAction(QIcon(":/images/hideBad.png"), tr("Toggle all bad channels"),this);
    m_pActionHideBad->setStatusTip(tr("Toggle all bad channels"));
    connect(m_pActionHideBad, &QAction::triggered,
            this, &RealTimeMultiSampleArrayWidget::hideBadChannels);
    addDisplayAction(m_pActionHideBad);
    m_pActionHideBad->setVisible(true);

    m_pActionQuickControl = new QAction(QIcon(":/images/quickControl.png"), tr("Show quick control widget"),this);
    m_pActionQuickControl->setStatusTip(tr("Show quick control widget"));
    connect(m_pActionQuickControl, &QAction::triggered,
            this, &RealTimeMultiSampleArrayWidget::showQuickControlWidget);
    addDisplayAction(m_pActionQuickControl);
    m_pActionQuickControl->setVisible(true);

    if(m_pTableView)
        delete m_pTableView;
    m_pTableView = new QTableView;

    //Install event filter for tracking mouse movements
    m_pTableView->viewport()->installEventFilter(this);
    m_pTableView->setMouseTracking(true);

    //set vertical layout
    QVBoxLayout *rtmsaLayout = new QVBoxLayout(this);

    rtmsaLayout->addWidget(m_pTableView);

    //set layouts
    this->setLayout(rtmsaLayout);

    init();
}


//*************************************************************************************************************

RealTimeMultiSampleArrayWidget::~RealTimeMultiSampleArrayWidget()
{
    //
    // Store Settings
    //
    if(!m_pRTMSA->getName().isEmpty()) {
        QString t_sRTMSAWName = m_pRTMSA->getName();

        QSettings settings;

        //Store scaling
        if(m_qMapChScaling.contains(FIFF_UNIT_T))
            settings.setValue(QString("RTMSAW/%1/scaleMAG").arg(t_sRTMSAWName), m_qMapChScaling[FIFF_UNIT_T]);

        if(m_qMapChScaling.contains(FIFF_UNIT_T_M))
            settings.setValue(QString("RTMSAW/%1/scaleGRAD").arg(t_sRTMSAWName), m_qMapChScaling[FIFF_UNIT_T_M]);

        if(m_qMapChScaling.contains(FIFFV_EEG_CH))
            settings.setValue(QString("RTMSAW/%1/scaleEEG").arg(t_sRTMSAWName), m_qMapChScaling[FIFFV_EEG_CH]);

        if(m_qMapChScaling.contains(FIFFV_EOG_CH))
            settings.setValue(QString("RTMSAW/%1/scaleEOG").arg(t_sRTMSAWName), m_qMapChScaling[FIFFV_EOG_CH]);

        if(m_qMapChScaling.contains(FIFFV_STIM_CH))
            settings.setValue(QString("RTMSAW/%1/scaleSTIM").arg(t_sRTMSAWName), m_qMapChScaling[FIFFV_STIM_CH]);

        if(m_qMapChScaling.contains(FIFFV_MISC_CH))
            settings.setValue(QString("RTMSAW/%1/scaleMISC").arg(t_sRTMSAWName), m_qMapChScaling[FIFFV_MISC_CH]);

        //Store filter
        if(m_pFilterWindow != 0) {
            FilterData filter = m_pFilterWindow->getUserDesignedFilter();

            settings.setValue(QString("RTMSAW/%1/filterHP").arg(t_sRTMSAWName), filter.m_dHighpassFreq);
            settings.setValue(QString("RTMSAW/%1/filterLP").arg(t_sRTMSAWName), filter.m_dLowpassFreq);
            settings.setValue(QString("RTMSAW/%1/filterOrder").arg(t_sRTMSAWName), filter.m_iFilterOrder);
            settings.setValue(QString("RTMSAW/%1/filterType").arg(t_sRTMSAWName), (int)filter.m_Type);
            settings.setValue(QString("RTMSAW/%1/filterDesignMethod").arg(t_sRTMSAWName), (int)filter.m_designMethod);
            settings.setValue(QString("RTMSAW/%1/filterTransition").arg(t_sRTMSAWName), filter.m_dParksWidth*(filter.m_sFreq/2));
            settings.setValue(QString("RTMSAW/%1/filterUserDesignActive").arg(t_sRTMSAWName), m_pFilterWindow->userDesignedFiltersIsActive());
        }

        //Store view
        settings.setValue(QString("RTMSAW/%1/viewZoomFactor").arg(t_sRTMSAWName), m_fZoomFactor);
        settings.setValue(QString("RTMSAW/%1/viewWindowSize").arg(t_sRTMSAWName), m_iT);
        if(m_pQuickControlWidget != 0)
            settings.setValue(QString("RTMSAW/%1/viewOpacity").arg(t_sRTMSAWName), m_pQuickControlWidget->getOpacityValue());

        //Store show/hide bad channel flag
        settings.setValue(QString("RTMSAW/%1/showHideBad").arg(t_sRTMSAWName), m_bHideBadChannels);

        //Store selected layout file
        if(m_pSelectionManagerWindow != 0)
            settings.setValue(QString("RTMSAW/%1/selectedLayoutFile").arg(t_sRTMSAWName), m_pSelectionManagerWindow->getCurrentLayoutFile());

        //Store show/hide bad channel flag
        if(m_pQuickControlWidget != 0)
            settings.setValue(QString("RTMSAW/%1/distanceTimeSpacerIndex").arg(t_sRTMSAWName), m_pQuickControlWidget->getDistanceTimeSpacerIndex());

        //Store signal and background colors
        if(m_pQuickControlWidget != 0) {
            settings.setValue(QString("RTMSAW/%1/signalColor").arg(t_sRTMSAWName), m_pQuickControlWidget->getSignalColor());
            settings.setValue(QString("RTMSAW/%1/backgroundColor").arg(t_sRTMSAWName), m_pQuickControlWidget->getBackgroundColor());
        }
    }
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::broadcastScaling(QMap<qint32,float> scaleMap)
{
    m_qMapChScaling = scaleMap;
    m_pRTMSAModel->setScaling(scaleMap);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::update(XMEASLIB::NewMeasurement::SPtr)
{
    if(!m_bInitialized)
    {
        if(m_pRTMSA->isChInit())
        {
            m_qListChInfo = m_pRTMSA->chInfo(); //ToDo Obsolete -> use fiffInfo instead
            m_pFiffInfo = m_pRTMSA->info();

            emit fiffFileUpdated(*m_pFiffInfo.data());

            m_fSamplingRate = m_pRTMSA->getSamplingRate();

            m_iMaxFilterTapSize = m_pRTMSA->getMultiSampleArray().at(m_pRTMSA->getMultiSampleArray().size()-1).cols();

            init();
        }
    }
    else
        m_pRTMSAModel->addData(m_pRTMSA->getMultiSampleArray());
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::init()
{
    if(m_qListChInfo.size() > 0)
    {
        QSettings settings;
        QString t_sRTMSAWName = m_pRTMSA->getName();

        //Init the model
        m_pRTMSAModel = RealTimeMultiSampleArrayModel::SPtr(new RealTimeMultiSampleArrayModel(this));

        m_pRTMSAModel->setFiffInfo(m_pFiffInfo);
        m_pRTMSAModel->setChannelInfo(m_qListChInfo);//ToDo Obsolete
        m_pRTMSAModel->setSamplingInfo(m_fSamplingRate, m_iT);

        //
        //-------- Init the delegate --------
        //
        m_pRTMSADelegate = RealTimeMultiSampleArrayDelegate::SPtr(new RealTimeMultiSampleArrayDelegate(this));
        m_pRTMSADelegate->initPainterPaths(m_pRTMSAModel.data());

        connect(this, &RealTimeMultiSampleArrayWidget::markerMoved,
                m_pRTMSADelegate.data(), &RealTimeMultiSampleArrayDelegate::markerMoved);

        //
        //-------- Init the view --------
        //
        m_pTableView->setModel(m_pRTMSAModel.data());
        m_pTableView->setItemDelegate(m_pRTMSADelegate.data());

        connect(m_pTableView, &QTableView::doubleClicked,
                m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::toggleFreeze);

        //set some size settings for m_pTableView
        m_pTableView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

        m_pTableView->setShowGrid(false);

        m_pTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); //Stretch 2 column to maximal width
        m_pTableView->horizontalHeader()->hide();
        m_pTableView->verticalHeader()->setDefaultSectionSize(m_fZoomFactor*m_fDefaultSectionSize);//Row Height

        m_pTableView->setAutoScroll(false);
        m_pTableView->setColumnHidden(0,true); //because content is plotted jointly with column=1
        m_pTableView->setColumnHidden(2,true);

        m_pTableView->resizeColumnsToContents();

        m_pTableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

//        connect(m_pTableView->verticalScrollBar(), &QScrollBar::valueChanged,
//                this, &RealTimeMultiSampleArrayWidget::visibleRowsChanged);

        if(settings.value(QString("RTMSAW/%1/showHideBad").arg(t_sRTMSAWName), false).toBool())
            hideBadChannels();

        //
        //-------- Init signal and background colors --------
        //
        QColor signalDefault = Qt::darkBlue;
        QColor backgroundDefault = Qt::white;
        QColor signal = settings.value(QString("RTMSAW/%1/signalColor").arg(t_sRTMSAWName), signalDefault).value<QColor>();
        QColor background = settings.value(QString("RTMSAW/%1/backgroundColor").arg(t_sRTMSAWName), backgroundDefault).value<QColor>();

        this->onTableViewBackgroundColorChanged(background);
        m_pRTMSADelegate->setSignalColor(signal);

        //
        //-------- Init context menu --------
        //
        m_pTableView->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(m_pTableView,SIGNAL(customContextMenuRequested(QPoint)),
                this,SLOT(channelContextMenu(QPoint)));

        //
        //-------- Init scaling --------
        //
        //Show only spin boxes and labels which type are present in the current loaded fiffinfo
        QList<FiffChInfo> channelList = m_pFiffInfo->chs;
        QList<int> availabeChannelTypes;

        for(int i = 0; i<channelList.size(); i++) {
            int unit = channelList.at(i).unit;
            int type = channelList.at(i).kind;

            if(!availabeChannelTypes.contains(unit))
                availabeChannelTypes.append(unit);

            if(!availabeChannelTypes.contains(type))
                availabeChannelTypes.append(type);
        }

//        if(!t_sRTMSAWName.isEmpty())
//        {
            m_qMapChScaling.clear();

            float val = 0.0f;
            if(availabeChannelTypes.contains(FIFF_UNIT_T)) {
                val = settings.value(QString("RTMSAW/%1/scaleMAG").arg(t_sRTMSAWName), 1e-11f).toFloat();
                m_qMapChScaling.insert(FIFF_UNIT_T, val);
            }

            if(availabeChannelTypes.contains(FIFF_UNIT_T_M)) {
                val = settings.value(QString("RTMSAW/%1/scaleGRAD").arg(t_sRTMSAWName), 1e-10f).toFloat();
                m_qMapChScaling.insert(FIFF_UNIT_T_M, val);
            }

            if(availabeChannelTypes.contains(FIFFV_EEG_CH)) {
                val = settings.value(QString("RTMSAW/%1/scaleEEG").arg(t_sRTMSAWName), 1e-4f).toFloat();
                m_qMapChScaling.insert(FIFFV_EEG_CH, val);
            }

            if(availabeChannelTypes.contains(FIFFV_EOG_CH)) {
                val = settings.value(QString("RTMSAW/%1/scaleEOG").arg(t_sRTMSAWName), 1e-3f).toFloat();
                m_qMapChScaling.insert(FIFFV_EOG_CH, val);
            }

            if(availabeChannelTypes.contains(FIFFV_STIM_CH)) {
                val = settings.value(QString("RTMSAW/%1/scaleSTIM").arg(t_sRTMSAWName), 1e-3f).toFloat();
                m_qMapChScaling.insert(FIFFV_STIM_CH, val);
            }

            if(availabeChannelTypes.contains(FIFFV_MISC_CH)) {
                val = settings.value(QString("RTMSAW/%1/scaleMISC").arg(t_sRTMSAWName), 1e-3f).toFloat();
                m_qMapChScaling.insert(FIFFV_MISC_CH, val);
            }

            m_pRTMSAModel->setScaling(m_qMapChScaling);
//        }

        //
        //-------- Init bad channel list --------
        //
        m_qListBadChannels.clear();
        for(int i = 0; i<m_pRTMSAModel->rowCount(); i++)
            if(m_pRTMSAModel->data(m_pRTMSAModel->index(i,2)).toBool())
                m_qListBadChannels << i;

        //-------- Init filter window --------
        m_pFilterWindow = QSharedPointer<FilterWindow>(new FilterWindow(this, Qt::Window/* | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint*/));
        //m_pFilterWindow->setWindowFlags(Qt::WindowStaysOnTopHint);

        m_pFilterWindow->setFiffInfo(m_pFiffInfo);
        m_pFilterWindow->setWindowSize(m_iMaxFilterTapSize);
        m_pFilterWindow->setMaxFilterTaps(m_iMaxFilterTapSize);

        connect(m_pFilterWindow.data(),static_cast<void (FilterWindow::*)(QString)>(&FilterWindow::applyFilter),
                m_pRTMSAModel.data(),static_cast<void (RealTimeMultiSampleArrayModel::*)(QString)>(&RealTimeMultiSampleArrayModel::setFilterChannelType));

        connect(m_pFilterWindow.data(), &FilterWindow::filterChanged,
                m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::filterChanged);

        connect(m_pFilterWindow.data(), &FilterWindow::filterActivated,
                m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::filterActivated);

        //Set stored filter settings from last session
        m_pFilterWindow->setFilterParameters(settings.value(QString("RTMSAW/%1/filterHP").arg(t_sRTMSAWName), 5.0).toDouble(),
                                                settings.value(QString("RTMSAW/%1/filterLP").arg(t_sRTMSAWName), 40.0).toDouble(),
                                                settings.value(QString("RTMSAW/%1/filterOrder").arg(t_sRTMSAWName), 128).toInt(),
                                                settings.value(QString("RTMSAW/%1/filterType").arg(t_sRTMSAWName), 2).toInt(),
                                                settings.value(QString("RTMSAW/%1/filterDesignMethod").arg(t_sRTMSAWName), 0).toInt(),
                                                settings.value(QString("RTMSAW/%1/filterTransition").arg(t_sRTMSAWName), 5.0).toDouble(),
                                                settings.value(QString("RTMSAW/%1/filterUserDesignActive").arg(t_sRTMSAWName), false).toBool());

        //
        //-------- Init channel selection manager --------
        //
        m_pChInfoModel = QSharedPointer<ChInfoModel>(new ChInfoModel(m_pFiffInfo.data(), this));

        m_pSelectionManagerWindow = SelectionManagerWindow::SPtr(new SelectionManagerWindow(this, m_pChInfoModel));
        //m_pSelectionManagerWindow->setWindowFlags(Qt::WindowStaysOnTopHint);

        connect(m_pSelectionManagerWindow.data(), &SelectionManagerWindow::showSelectedChannelsOnly,
                this, &RealTimeMultiSampleArrayWidget::showSelectedChannelsOnly);

        //Connect channel info model
        connect(m_pSelectionManagerWindow.data(), &SelectionManagerWindow::loadedLayoutMap,
                m_pChInfoModel.data(), &ChInfoModel::layoutChanged);

        connect(m_pSelectionManagerWindow.data(), &SelectionManagerWindow::loadedLayoutMap,
                m_pSelectionManagerWindow.data(), &SelectionManagerWindow::updateBadChannels);

        connect(m_pChInfoModel.data(), &ChInfoModel::channelsMappedToLayout,
                m_pSelectionManagerWindow.data(), &SelectionManagerWindow::setCurrentlyMappedFiffChannels);

        m_pChInfoModel->fiffInfoChanged(m_pFiffInfo.data());

        m_pSelectionManagerWindow->setCurrentLayoutFile(settings.value(QString("RTMSAW/%1/selectedLayoutFile").arg(t_sRTMSAWName), "babymeg-mag-inner-layer.lout").toString());

        //
        //-------- Init quick control widget --------
        //
        QStringList slFlags = m_pRTMSA->getDisplayFlags();

        #ifdef BUILD_BASIC_MNEX_VERSION
            std::cout<<"BUILD_BASIC_MNEX_VERSION Defined"<<std::endl;
            slFlags.clear();
            slFlags << "projections" << "view" << "scaling";
        #endif

        m_pQuickControlWidget = QSharedPointer<QuickControlWidget>(new QuickControlWidget(m_qMapChScaling, m_pFiffInfo, "RT Display", slFlags));

        m_pQuickControlWidget->setWindowFlags(Qt::WindowStaysOnTopHint);

        //Handle scaling
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::scalingChanged,
                this, &RealTimeMultiSampleArrayWidget::broadcastScaling);

        //Handle signal color changes
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::signalColorChanged,
                m_pRTMSADelegate.data(), &RealTimeMultiSampleArrayDelegate::setSignalColor);

        //Handle background color changes
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::backgroundColorChanged,
                this, &RealTimeMultiSampleArrayWidget::onTableViewBackgroundColorChanged);

        //Handle screenshot signals
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::makeScreenshot,
                this, &RealTimeMultiSampleArrayWidget::onMakeScreenshot);

        //Handle projections
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::projSelectionChanged,
                this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::updateProjection);

        //Handle compensators
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::compSelectionChanged,
                this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::updateCompensator);

        //Handle SPHARA
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::spharaActivationChanged,
                this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::updateSpharaActivation);

        connect(m_pQuickControlWidget.data(), &QuickControlWidget::spharaOptionsChanged,
                this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::updateSpharaOptions);

        //Handle view changes
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::zoomChanged,
                this, &RealTimeMultiSampleArrayWidget::zoomChanged);

        connect(m_pQuickControlWidget.data(), &QuickControlWidget::timeWindowChanged,
                this, &RealTimeMultiSampleArrayWidget::timeWindowChanged);

        //Handle Filtering
        connect(m_pFilterWindow.data(), &FilterWindow::activationCheckBoxListChanged,
                m_pQuickControlWidget.data(), &QuickControlWidget::filterGroupChanged);

        connect(m_pQuickControlWidget.data(), &QuickControlWidget::showFilterOptions,
                this, &RealTimeMultiSampleArrayWidget::showFilterWidget);

        //Handle trigger detection
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::triggerInfoChanged,
                this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::triggerInfoChanged);

        connect(m_pQuickControlWidget.data(), &QuickControlWidget::resetTriggerCounter,
                this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::resetTriggerCounter);

        connect(this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::triggerDetected,
                m_pQuickControlWidget.data(), &QuickControlWidget::setNumberDetectedTriggers);

        //Handle time spacer distance
        connect(m_pQuickControlWidget.data(), &QuickControlWidget::distanceTimeSpacerChanged,
                this->m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::distanceTimeSpacerChanged);

        m_pQuickControlWidget->filterGroupChanged(m_pFilterWindow->getActivationCheckBoxList());

        m_pQuickControlWidget->setViewParameters(settings.value(QString("RTMSAW/%1/viewZoomFactor").arg(t_sRTMSAWName), 1.0).toFloat(),
                                                 settings.value(QString("RTMSAW/%1/viewWindowSize").arg(t_sRTMSAWName), 10).toInt(),
                                                 settings.value(QString("RTMSAW/%1/viewOpacity").arg(t_sRTMSAWName), 95).toInt());

        m_pQuickControlWidget->setDistanceTimeSpacerIndex(settings.value(QString("RTMSAW/%1/distanceTimeSpacerIndex").arg(t_sRTMSAWName), 3).toInt());

        m_pQuickControlWidget->setSignalBackgroundColors(signal, background);

        //If projections are wanted activate projections as default
        if(slFlags.contains("projections")) {
            m_pRTMSAModel->updateProjection();
        }

        //Initialized
        m_bInitialized = true;
    }
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::channelContextMenu(QPoint pos)
{
    //obtain index where index was clicked
    QModelIndex index = m_pTableView->indexAt(pos);

    //get selected items
    QModelIndexList selected = m_pTableView->selectionModel()->selectedIndexes();

    //create custom context menu and actions
    QMenu *menu = new QMenu(this);

    //**************** Marking ****************
    if(!m_qListBadChannels.contains(index.row())) {
        QAction* doMarkChBad = menu->addAction(tr("Mark as bad"));
        connect(doMarkChBad, &QAction::triggered,
                this, &RealTimeMultiSampleArrayWidget::markChBad);
    } else {
        QAction* doMarkChGood = menu->addAction(tr("Mark as good"));
        connect(doMarkChGood, &QAction::triggered,
                this, &RealTimeMultiSampleArrayWidget::markChBad);
    }

    // non C++11 alternative
    m_qListCurrentSelection.clear();
    for(qint32 i = 0; i < selected.size(); ++i)
        if(selected[i].column() == 1)
            m_qListCurrentSelection.append(m_pRTMSAModel->getIdxSelMap()[selected[i].row()]);

    QAction* doSelection = menu->addAction(tr("Apply selection"));
    connect(doSelection, &QAction::triggered,
            this, &RealTimeMultiSampleArrayWidget::applySelection);

    //select channels
    QAction* hideSelection = menu->addAction(tr("Hide selection"));
    connect(hideSelection, &QAction::triggered, this,
            &RealTimeMultiSampleArrayWidget::hideSelection);

    //undo selection
    QAction* resetAppliedSelection = menu->addAction(tr("Reset selection"));
    connect(resetAppliedSelection, &QAction::triggered,
            m_pRTMSAModel.data(), &RealTimeMultiSampleArrayModel::resetSelection);
    connect(resetAppliedSelection, &QAction::triggered,
            this, &RealTimeMultiSampleArrayWidget::resetSelection);

    //show context menu
    menu->popup(m_pTableView->viewport()->mapToGlobal(pos));
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::resizeEvent(QResizeEvent* resizeEvent)
{
    Q_UNUSED(resizeEvent)
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::keyPressEvent(QKeyEvent* keyEvent)
{
    Q_UNUSED(keyEvent)
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::mousePressEvent(QMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent)
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent)
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent)
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent)
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::wheelEvent(QWheelEvent* wheelEvent)
{
    Q_UNUSED(wheelEvent)
}


//*************************************************************************************************************

bool RealTimeMultiSampleArrayWidget::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_pTableView->viewport() && event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        emit markerMoved(mouseEvent->pos(), m_pTableView->rowAt(mouseEvent->pos().y()));
        return true;
    }

    return NewMeasurementWidget::eventFilter(object, event);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::zoomChanged(double zoomFac)
{
    m_fZoomFactor = zoomFac;

    m_pTableView->verticalHeader()->setDefaultSectionSize(m_fZoomFactor*m_fDefaultSectionSize);//Row Height
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::timeWindowChanged(int T)
{
    m_iT = T;

    m_pRTMSAModel->setSamplingInfo(m_fSamplingRate, T);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::applySelection()
{
    //Hide non selected channels/rows in the data views
    for(int i = 0; i<m_pRTMSAModel->rowCount(); i++) {
        //if channel is a bad channel and bad channels are to be hidden -> do not show
        if(m_qListCurrentSelection.contains(i))
            m_pTableView->showRow(i);
        else
            m_pTableView->hideRow(i);
    }

    //Update the visible channel list which are to be filtered
    //visibleRowsChanged(0);

    //m_pRTMSAModel->selectRows(m_qListCurrentSelection);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::hideSelection()
{
    for(int i=0; i<m_qListCurrentSelection.size(); i++)
        m_pTableView->hideRow(m_qListCurrentSelection.at(i));

    //Update the visible channel list which are to be filtered
    //visibleRowsChanged(0);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::resetSelection()
{
    // non C++11 alternative
    for(qint32 i = 0; i < m_qListChInfo.size(); ++i) {
        if(m_qListBadChannels.contains(i)) {
            if(!m_bHideBadChannels) {
                m_pTableView->showRow(i);
            }
        }
        else {
            m_pTableView->showRow(i);
        }
    }

    //Update the visible channel list which are to be filtered
    //visibleRowsChanged(0);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::showSelectedChannelsOnly(QStringList selectedChannels)
{
    m_slSelectedChannels = selectedChannels;

    //Hide non selected channels/rows in the data views
    for(int i = 0; i<m_pRTMSAModel->rowCount(); i++) {
        QString channel = m_pRTMSAModel->data(m_pRTMSAModel->index(i, 0), Qt::DisplayRole).toString();

        //if channel is a bad channel and bad channels are to be hidden -> do not show
        if(!selectedChannels.contains(channel) || (m_qListBadChannels.contains(i) && m_bHideBadChannels))
            m_pTableView->hideRow(i);
        else
            m_pTableView->showRow(i);
    }

    //Update the visible channel list which are to be filtered
    //visibleRowsChanged(0);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::visibleRowsChanged(int value)
{
    Q_UNUSED(value);
    //std::cout <<"Visible channels: "<< m_pTableView->rowAt(0) << "-" << m_pTableView->rowAt(m_pTableView->height())<<std::endl;

    int from = m_pTableView->rowAt(0);
    if(from != 0)
        from--;

    int to = m_pTableView->rowAt(m_pTableView->height()-1);
    if(to != m_pRTMSAModel->rowCount()-1)
        to++;

    if(from > to)
        to = m_pRTMSAModel->rowCount()-1;

    QStringList channelNames;

    for(int i = from; i<=to; i++) {
        channelNames << m_pRTMSAModel->data(m_pRTMSAModel->index(i, 0), Qt::DisplayRole).toString();
        //std::cout << m_pRTMSAModel->data(m_pRTMSAModel->index(i, 0), Qt::DisplayRole).toString().toStdString() << std::endl;
    }

    m_pRTMSAModel->createFilterChannelList(channelNames);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::markChBad()
{
    QModelIndexList selected = m_pTableView->selectionModel()->selectedIndexes();

    for(int i=0; i<selected.size(); i++) {
        if(m_qListBadChannels.contains(selected[i].row())) { //mark as good
            m_pRTMSAModel->markChBad(selected[i], false);
            m_qListBadChannels.removeAll(selected[i].row());
        }
        else {
            m_pRTMSAModel->markChBad(selected[i], true);
            m_qListBadChannels.append(selected[i].row());
        }
    }

    m_pRTMSAModel->updateProjection();    

    m_pSelectionManagerWindow->updateBadChannels();
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::hideBadChannels()
{
    if(m_pActionHideBad->toolTip() == "Show all bad channels") {
        m_pActionHideBad->setIcon(QIcon(":/images/hideBad.png"));
        m_pActionHideBad->setToolTip("Hide all bad channels");
        m_pActionHideBad->setStatusTip(tr("Hide all bad channels"));
        m_bHideBadChannels = false;
    }
    else {
        m_pActionHideBad->setIcon(QIcon(":/images/showBad.png"));
        m_pActionHideBad->setToolTip("Show all bad channels");
        m_pActionHideBad->setStatusTip(tr("Show all bad channels"));
        m_bHideBadChannels = true;
    }

    //Hide non selected channels/rows in the data views
    for(int i = 0; i<m_qListBadChannels.size(); i++) {
        if(m_bHideBadChannels)
            m_pTableView->hideRow(m_qListBadChannels.at(i));
        else
            m_pTableView->showRow(m_qListBadChannels.at(i));
    }

    //Update the visible channel list which are to be filtered
    //visibleRowsChanged(0);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::onTriggerDetected(int numberDetectedTriggers)
{
    m_pQuickControlWidget->setNumberDetectedTriggers(numberDetectedTriggers);
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::showFilterWidget(bool state)
{
    if(state) {
        if(m_pFilterWindow->isActiveWindow())
            m_pFilterWindow->hide();
        else {
            m_pFilterWindow->activateWindow();
            m_pFilterWindow->show();
        }
    } else {
        m_pFilterWindow->hide();
    }
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::showSensorSelectionWidget()
{
    if(m_pSelectionManagerWindow->isActiveWindow())
        m_pSelectionManagerWindow->hide();
    else {
        m_pSelectionManagerWindow->activateWindow();
        m_pSelectionManagerWindow->show();
    }
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::showQuickControlWidget()
{
    m_pQuickControlWidget->show();
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::onTableViewBackgroundColorChanged(const QColor& backgroundColor)
{
    m_pTableView->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(backgroundColor.red()).arg(backgroundColor.green()).arg(backgroundColor.blue()));
}


//*************************************************************************************************************

void RealTimeMultiSampleArrayWidget::onMakeScreenshot(const QString& imageType)
{
    // Create file name
    QString sDate = QDate::currentDate().toString("yyyy_MM_dd");
    QString sTime = QTime::currentTime().toString("hh_mm_ss");

    if(!QDir("./Screenshots").exists()) {
        QDir().mkdir("./Screenshots");
    }

    if(imageType.contains("SVG"))
    {
        QString fileName = QString("./Screenshots/%1-%2-DataView.svg").arg(sDate).arg(sTime);
        // Generate screenshot
        QSvgGenerator svgGen;
        svgGen.setFileName(fileName);
        svgGen.setSize(m_pTableView->size());
        svgGen.setViewBox(m_pTableView->rect());

        m_pTableView->render(&svgGen);
    }

    if(imageType.contains("PNG"))
    {
        QString fileName = QString("./Screenshots/%1-%2-DataView.png").arg(sDate).arg(sTime);
        QPixmap pixMap = QPixmap::grabWidget(m_pTableView);
        pixMap.save(fileName);
    }
}
