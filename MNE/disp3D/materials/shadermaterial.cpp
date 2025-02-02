//=============================================================================================================
/**
* @file     shadermaterial.cpp
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     Februaray, 2016
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
* @brief    ShaderMaterial class definition
*/


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "shadermaterial.h"


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace DISP3DLIB;
using namespace Qt3DRender;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

ShaderMaterial::ShaderMaterial(QNode *parent)
: QMaterial(parent)
, m_vertexEffect(new QEffect())
, m_ambientParameter(new QParameter(QStringLiteral("ka"), QColor::fromRgbF(0.05f, 0.05f, 0.05f, 1.0f)))
, m_diffuseParameter(new QParameter(QStringLiteral("kd"), QColor::fromRgbF(0.7f, 0.7f, 0.7f, 1.0f)))
, m_specularParameter(new QParameter(QStringLiteral("ks"), QColor::fromRgbF(0.1f, 0.1f, 0.1f, 1.0f)))
, m_shininessParameter(new QParameter(QStringLiteral("shininess"), 1.0f))
, m_alphaParameter(new QParameter("alpha", 0.5f))
, m_vertexGL3Technique(new QTechnique())
, m_vertexGL3RenderPass(new QRenderPass())
, m_vertexGL3Shader(new QShaderProgram())
{
    this->init();
}


//*************************************************************************************************************

ShaderMaterial::~ShaderMaterial()
{
    delete m_vertexEffect;
    delete m_ambientParameter;
    delete m_diffuseParameter;
    delete m_specularParameter;
    delete m_specularParameter;
    delete m_shininessParameter;
    delete m_alphaParameter;
    delete m_vertexGL3Technique;
    delete m_vertexGL3RenderPass;
    delete m_vertexGL3Shader;
}


//*************************************************************************************************************

void ShaderMaterial::init()
{
//    m_vertexGL3Shader->setVertexShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/materials/shaders/gl3/brain.vert"))));
//    m_vertexGL3Shader->setFragmentShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/materials/shaders/gl3/brain.frag"))));

    m_vertexGL3Shader->setVertexShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/materials/shaders/gl3/pervertexphongalpha.vert"))));
    m_vertexGL3Shader->setFragmentShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/materials/shaders/gl3/pervertexphongalpha.frag"))));

    m_vertexGL3Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
    m_vertexGL3Technique->graphicsApiFilter()->setMajorVersion(3);
    m_vertexGL3Technique->graphicsApiFilter()->setMinorVersion(1);
    m_vertexGL3Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);

    m_vertexGL3RenderPass->setShaderProgram(m_vertexGL3Shader);

    QBlendState* pBlendState = new QBlendState();
    pBlendState->setSrcRGB(QBlendState::SrcAlpha);
    pBlendState->setDstRGB(QBlendState::OneMinusSrcAlpha);

    QCullFace* pCullFace = new QCullFace();
    pCullFace->setMode(QCullFace::Back);

    QDepthTest* pDepthTest = new QDepthTest();
    pDepthTest->setFunc(QDepthTest::Less);

    QDepthMask* pDepthMask = new QDepthMask();
    pDepthMask->setMask(true);

    QBlendEquation* pBlendEquation = new QBlendEquation();
    pBlendEquation->setMode(QBlendEquation::FuncAdd);

    m_vertexGL3RenderPass->addRenderState(pBlendState);
    m_vertexGL3RenderPass->addRenderState(pCullFace);
    m_vertexGL3RenderPass->addRenderState(pDepthTest);
    m_vertexGL3RenderPass->addRenderState(pDepthMask);
    m_vertexGL3RenderPass->addRenderState(pBlendEquation);

    m_vertexGL3Technique->addPass(m_vertexGL3RenderPass);

    m_vertexEffect->addTechnique(m_vertexGL3Technique);

    m_vertexEffect->addParameter(m_ambientParameter);
    m_vertexEffect->addParameter(m_diffuseParameter);
    m_vertexEffect->addParameter(m_specularParameter);
    m_vertexEffect->addParameter(m_shininessParameter);
    m_vertexEffect->addParameter(m_alphaParameter);

    this->setEffect(m_vertexEffect);
}


//*************************************************************************************************************

float ShaderMaterial::alpha()
{
    return m_alphaParameter->value().toFloat();
}


//*************************************************************************************************************

void ShaderMaterial::setAlpha(float alpha)
{
    m_alphaParameter->setValue(alpha);
}

