// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2022 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Heba Eiz <heba.eiz@zaha-hadid.com> , Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//



#ifndef ZSPACE_OBJ_COMPUTEMESH_H
#define ZSPACE_OBJ_COMPUTEMESH_H



#pragma once
#include <vector>
#include <string>
using namespace std;

#include "zInterface/objects/zObj.h"
#include "zInterface/objects/zObjParticle.h"

//#include "zInterface/functionsets/zFnParticle.h"


namespace  zSpace
{

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjComputeMesh
	*	\brief The mesh compute object class top breif.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zObjComputeMesh : public zObj
	{

	public:

		/*!	\brief check if zObjCompute is valid  */
		bool isValid = false;

		/*!	\brief container of vertex positions  */
		zPoint* vertexPositions;

		/*!	\brief 2D container of polygon vertex connectivity  */
		int** polygons;

		/*!	\brief 1D container of number of vertices per polygon */
		int* polyCounts;

		zObjParticle* o_Particles;

		/*!	\brief 2D container of connected vertice per vertex  */
		int** cVertices;

		/*!	\brief 1D container of number of connected vertice per vertex  */
		int* cVerticesCounts;

		/*!	\brief 2D container of edge vertex connectivity  */
		int** edges;

		/*!	\brief 2D container of triangle vertex connectivity per polygon */
		int** triangles;

		/*!	\brief 1D container of number of triangles per polygon */
		int* triCounts;

		/*!	\brief number of vertices */
		int nV;

		/*!	\brief number of edges */
		int nE;

		/*!	\brief number of polygons */
		int nF;

		/*!	\brief number of triangles */
		int nT;

		/*!	\brief time step variable. Should be between 0 & 1. */
		double dT;

		/*!	\brief A container of booleans to indicate if the vertex is fixed or not. */
		bool* vFixed;
	

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjComputeMesh();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjComputeMesh();


		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------
		void draw() override;
		void getBounds(zPoint& minBB, zPoint& maxBB) override;

		

	};

}
#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterface/objects/zObjComputeMesh.cpp>
#endif

#endif