//=============================================================================================================
/**
* @file     brainrtconnectivitydatatreeitem.cpp
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     January, 2016
*
* @section  LICENSE
*
* Copyright (C) 2016, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    BrainRTConnectivityDataTreeItem class definition.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "brainrtconnectivitydatatreeitem.h"


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace Eigen;
using namespace MNELIB;
using namespace DISP3DLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

BrainRTConnectivityDataTreeItem::BrainRTConnectivityDataTreeItem(int iType, const QString &text)
: AbstractTreeItem(iType, text)
, m_bIsInit(false)
{
    this->setEditable(false);
    this->setToolTip("Real time connectivity data");
}


//*************************************************************************************************************

BrainRTConnectivityDataTreeItem::~BrainRTConnectivityDataTreeItem()
{
}


//*************************************************************************************************************

QVariant BrainRTConnectivityDataTreeItem::data(int role) const
{
    return AbstractTreeItem::data(role);
}


//*************************************************************************************************************

void  BrainRTConnectivityDataTreeItem::setData(const QVariant& value, int role)
{
    AbstractTreeItem::setData(value, role);
}


//*************************************************************************************************************

bool BrainRTConnectivityDataTreeItem::init(const MNEForwardSolution& tForwardSolution, int iHemi)
{   
    //Set hemisphere information as item's data
    this->setData(iHemi, BrainRTConnectivityDataTreeItemRoles::RTHemi);

    //Set data based on clusterd or full source space
    bool isClustered = tForwardSolution.src[iHemi].isClustered();

    QVariant data;

    //Add meta information as item children
    QString sIsClustered = isClustered ? "Clustered" : "Full";
    BrainTreeMetaItem* pItemSourceSpaceType = new BrainTreeMetaItem(BrainTreeMetaItemTypes::RTDataSourceSpaceType, sIsClustered);
    pItemSourceSpaceType->setEditable(false);
    *this<<pItemSourceSpaceType;
    data.setValue(sIsClustered);
    pItemSourceSpaceType->setData(data, BrainTreeMetaItemRoles::RTDataSourceSpaceType);

    m_bIsInit = true;

    return true;
}


//*************************************************************************************************************

bool BrainRTConnectivityDataTreeItem::addData(const MatrixXd& matNewConnection)
{
    Q_UNUSED(matNewConnection)
    if(!m_bIsInit) {
        qDebug()<<"BrainRTConnectivityDataTreeItem::updateData - Rt Item has not been initialized yet!";
        return false;
    }

    return true;
}

