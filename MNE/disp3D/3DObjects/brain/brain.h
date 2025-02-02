//=============================================================================================================
/**
* @file     brain.h
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     November, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    Brain class declaration
*
*/

#ifndef BRAIN_H
#define BRAIN_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "../../disp3D_global.h"

#include "braintreemodel.h"

#include <fs/surfaceset.h>
#include <fs/annotationset.h>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QSharedPointer>

#include <Qt3DCore/QEntity>


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE DISP3DLIB
//=============================================================================================================

namespace DISP3DLIB
{


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================


//=============================================================================================================
/**
* Visualizes a brain in 3D.
*
* @brief Visualizes a brain in 3D.
*/
class DISP3DNEWSHARED_EXPORT Brain : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    typedef QSharedPointer<Brain> SPtr;             /**< Shared pointer type for Brain class. */
    typedef QSharedPointer<const Brain> ConstSPtr;  /**< Const shared pointer type for Brain class. */

    //=========================================================================================================
    /**
    * Default constructor.
    *
    * @param[in] parent         The parent of this class.
    */
    Brain(QEntity * parent = 0);

    //=========================================================================================================
    /**
    * Default destructor.
    */
    ~Brain();

    //=========================================================================================================
    /**
    * Adds FreeSurfer brain data SETS.
    *
    * @param[in] text               The name of the surface set to which the data is to be added.
    * @param[in] tSurfaceSet        FreeSurfer surface set.
    * @param[in] tAnnotationSet     FreeSurfer annotation set.
    *
    * @return                       Returns true if successful.
    */
    bool addData(const QString& text, const FSLIB::SurfaceSet& tSurfaceSet, const FSLIB::AnnotationSet& tAnnotationSet);

    //=========================================================================================================
    /**
    * Adds FreeSurfer brain data.
    *
    * @param[in] text               The name of the surface set to which the data is to be added.
    * @param[in] pSurface           FreeSurfer surface.
    * @param[in] pAnnotation        FreeSurfer annotation.
    *
    * @return                       Returns true if successful.
    */
    bool addData(const QString& text, const FSLIB::Surface& tSurface, const FSLIB::Annotation& tAnnotation);

    //=========================================================================================================
    /**
    * Adds source space brain data.
    *
    * @param[in] text               The name of the surface set to which the data is to be added.
    * @param[in] tSourceSpace       The source space information.
    *
    * @return                       Returns true if successful.
    */
    bool addData(const QString& text, const MNELIB::MNESourceSpace& tSourceSpace);

    //=========================================================================================================
    /**
    * Adds source estimated activation data.
    *
    * @param[in] text               The name of the surface set to which the actiavtion data is to be added.
    * @param[in] tSourceEstimate    The MNESourceEstimate.
    * @param[in] tForwardSolution   The MNEForwardSolution.
    *
    * @return                       Returns a list with the tree items which now hold the activation data. Use this list to update the data, i.e. during real time applications.
    */
    QList<BrainRTSourceLocDataTreeItem*> addData(const QString& text, const MNELIB::MNESourceEstimate& tSourceEstimate, const MNELIB::MNEForwardSolution& tForwardSolution = MNELIB::MNEForwardSolution());

    //=========================================================================================================
    /**
    * Return the brain tree model.
    *
    * @return returns a pointer to the brain tree model BrainTreeModel.
    */
    BrainTreeModel* getBrainTreeModel();    

protected:
    BrainTreeModel*     m_pBrainTreeModel;  /**< The model which holds all of the loaded brain data. */
};

} // NAMESPACE

#endif // BRAIN_H
