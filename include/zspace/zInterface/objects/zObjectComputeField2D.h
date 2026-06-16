// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>, Leo Bieling <leo.bieling@zaha-hadid.com>
//

#ifndef ZSPACE_OBJECT_COMPUTE_FIELD2D_H
#define ZSPACE_OBJECT_COMPUTE_FIELD2D_H

#pragma once

#include <zspace/zInterface/objects/zObject.h>

namespace zSpace
{
	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjectComputeField2D
	*	\brief The mesh object class.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/
	
	class ZSPACE_API zObjectComputeField2D : public zObject
	{
	protected:
		/*!	\brief container of vertex positions  */
		zPoint* vertexPositions;

		/*!	\brief container of polygons (quads)  */
		int* polygons;

		/*!	\brief container of edges  */
		int* edges;

		/*!	\brief number of vertices */
		int nV;

		/*!	\brief number of edges */
		int nE;

		/*!	\brief number of polygons */
		int nF;

	public:

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------
		
		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjectComputeField2D();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjectComputeField2D();

	};
		

}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/objects/zObjectComputeField2D.cpp>
#endif

#endif
