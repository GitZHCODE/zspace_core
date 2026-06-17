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

#ifndef ZSPACE_OBJECT_PARTICLE_H
#define ZSPACE_OBJECT_PARTICLE_H

#pragma once

#include <zspace/zInterface/objects/zObject.h>
#include <zspace/zCore/dynamics/zParticle.h>


namespace zSpace
{
	class zFnParticle;
	class zFnComputeMesh;
	class zFnGraphDynamics;
	class zFnMeshDynamics;

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjectParticle
	*	\brief The particle object class.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zObjectParticle :public zObject
	{
		friend class zFnParticle;
		friend class zFnComputeMesh;
		friend class zFnGraphDynamics;
		friend class zFnMeshDynamics;

	private:
		//--------------------------
		//---- PRIVATE ATTRIBUTES
		//--------------------------

		/*! \brief particle */
		zParticle particle;

	public:

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------
		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjectParticle();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjectParticle();

		//--------------------------

	};


	/** \addtogroup zCore
	*	\brief The core datastructures of the library.
	*  @{
	*/

	/** \addtogroup zBase
	*	\brief  The base classes, enumerators ,defintions of the library.
	*  @{
	*/

	/** \addtogroup zTypeDefs
	*	\brief  The type defintions of the library.
	*  @{
	*/

	/** \addtogroup Container
	*	\brief  The container typedef of the library.
	*  @{
	*/

	/*! \typedef zObjectParticleArray
	*	\brief A vector of zObjectParticle.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectParticle> zObjectParticleArray;

	/*! \brief Transitional keyword-first public particle object name. */

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/objects/zObjectParticle.cpp>
#endif

#endif
