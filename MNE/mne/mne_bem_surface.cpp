//=============================================================================================================
/**
* @file     mne_bem_surface.cpp
* @author   Jana Kiesel<jana.kiesel@tu-ilmenau.de>;
*           Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     June, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, Jana Kiesel, Christoph Dinh and Matti Hamalainen. All rights reserved.
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
* @brief     MNEBemSurface class implementation.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "mne_bem_surface.h"
#include <fstream>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNELIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

MNEBemSurface::MNEBemSurface()
: id(-1)
, np(-1)
, ntri(-1)
, coord_frame(-1)
, sigma(-1)
, rr(MatrixX3f::Zero(0,3))
, nn(MatrixX3f::Zero(0,3))
, tris(MatrixX3i::Zero(0,3))
, tri_cent(MatrixX3d::Zero(0,3))
, tri_nn(MatrixX3d::Zero(0,3))
, tri_area(VectorXd::Zero(0))
{
}


//*************************************************************************************************************

MNEBemSurface::MNEBemSurface(const MNEBemSurface& p_MNEBemSurface)
: id(p_MNEBemSurface.id)
, np(p_MNEBemSurface.np)
, ntri(p_MNEBemSurface.ntri)
, coord_frame(p_MNEBemSurface.coord_frame)
, sigma(p_MNEBemSurface.sigma)
, rr(p_MNEBemSurface.rr)
, nn(p_MNEBemSurface.nn)
, tris(p_MNEBemSurface.tris)
, tri_cent(p_MNEBemSurface.tri_cent)
, tri_nn(p_MNEBemSurface.tri_nn)
, tri_area(p_MNEBemSurface.tri_area)
, neighbor_tri(p_MNEBemSurface.neighbor_tri)
, neighbor_vert(p_MNEBemSurface.neighbor_vert)
{
    //*m_pGeometryData = *p_MNEBemSurface.m_pGeometryData;
}


//*************************************************************************************************************

MNEBemSurface::~MNEBemSurface()
{

}


//*************************************************************************************************************

void MNEBemSurface::clear()
{
    id = -1;
    np = -1;
    ntri = -1;
    coord_frame = -1;
    sigma = -1;
    rr = MatrixX3f::Zero(0,3);
    nn = MatrixX3f::Zero(0,3);
    tris = MatrixX3i::Zero(0,3);
    tri_cent = MatrixX3d::Zero(0,3);
    tri_nn = MatrixX3d::Zero(0,3);
    tri_area = VectorXd::Zero(0);
}


//*************************************************************************************************************

bool MNEBemSurface::addTriangleData()
{
    //
    //   Main triangulation
    //
    printf("\tCompleting triangulation info...");
    this->tri_cent = MatrixX3d::Zero(this->ntri,3);
    this->tri_nn = MatrixX3d::Zero(this->ntri,3);
    this->tri_area = VectorXd::Zero(this->ntri);

    Matrix3d r;
    Vector3d a, b;
    int k = 0;
    float size = 0;
    for (qint32 i = 0; i < this->ntri; ++i)
    {
        for ( qint32 j = 0; j < 3; ++j)
        {
            k = this->tris(i, j);

            r(j,0) = this->rr(k, 0);
            r(j,1) = this->rr(k, 1);
            r(j,2) = this->rr(k, 2);

            this->tri_cent(i, 0) += this->rr(k, 0);
            this->tri_cent(i, 1) += this->rr(k, 1);
            this->tri_cent(i, 2) += this->rr(k, 2);
        }
        this->tri_cent.row(i) /= 3.0f;

        //cross product {cross((r2-r1),(r3-r1))}
        a = r.row(1) - r.row(0 );
        b = r.row(2) - r.row(0);
        this->tri_nn(i,0) = a(1)*b(2)-a(2)*b(1);
        this->tri_nn(i,1) = a(2)*b(0)-a(0)*b(2);
        this->tri_nn(i,2) = a(0)*b(1)-a(1)*b(0);

        //area
        size = this->tri_nn.row(i)*this->tri_nn.row(i).transpose();
        size = std::pow(size, 0.5f );

        this->tri_area(i) = size/2.0f;
        this->tri_nn.row(i) /= size;


    }

    std::fstream doc("./Output/tri_area.dat", std::ofstream::out | std::ofstream::trunc);
    if(doc)  // if succesfully opened
    {
      // instructions
      doc << this->tri_area << "\n";
      doc.close();
    }

    printf("[done]\n");

//        qDebug() << "this->tri_cent:" << this->tri_cent(0,0) << this->tri_cent(0,1) << this->tri_cent(0,2);
//        qDebug() << "this->tri_cent:" << this->tri_cent(2,0) << this->tri_cent(2,1) << this->tri_cent(2,2);

        qDebug() << "this->tri_nn:" << this->tri_nn(0,0) << this->tri_nn(0,1) << this->tri_nn(0,2);
        qDebug() << "this->tri_nn:" << this->tri_nn(2,0) << this->tri_nn(2,1) << this->tri_nn(2,2);

    return true;
}


//*************************************************************************************************************

bool MNEBemSurface::addVertexNormals()
{

      //
      //   Accumulate the vertex normals
      //

        for (qint32 p = 0; p < this->ntri; p++)         //check each triangle
        {
            for (qint32 j=0; j<3 ; j++)
            {
                int nodenr;
                nodenr = this->tris(p,j);               //find the corners(nodes) of the triangles
                this->nn(nodenr,0) += this->tri_nn(p,0);  //add the triangle normal to the nodenormal
                this->nn(nodenr,1) += this->tri_nn(p,1);
                this->nn(nodenr,2) += this->tri_nn(p,2);
            }
        }

            // normalize
        for (qint32 p = 0; p < this->np; p++)
        {
            float size = 0;
            size = this->nn.row(p)*this->nn.row(p).transpose();
            size = std::pow(size, 0.5f );
            this->nn.row(p) /= size;
        }

return true;
}


//*************************************************************************************************************

void MNEBemSurface::writeToStream(FiffStream *p_pStream)
{
    if(this->id <=0)
        this->id=FIFFV_MNE_SURF_UNKNOWN;
    if(this->sigma>0.0)
        p_pStream->write_float(FIFF_BEM_SIGMA, &this->sigma);
    p_pStream->write_int(FIFF_BEM_SURF_ID, &this->id);
    p_pStream->write_int(FIFF_MNE_COORD_FRAME, &this->coord_frame);
    p_pStream->write_int(FIFF_BEM_SURF_NNODE, &this->np);
    p_pStream->write_int(FIFF_BEM_SURF_NTRI, &this->ntri);
    p_pStream->write_float_matrix(FIFF_BEM_SURF_NODES, this->rr);
    if (this->ntri > 0)
        p_pStream->write_int_matrix(FIFF_BEM_SURF_TRIANGLES, this->tris.array() + 1);
    p_pStream->write_float_matrix(FIFF_BEM_SURF_NORMALS, this->nn);
}


