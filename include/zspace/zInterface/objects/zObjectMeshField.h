// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//

#ifndef ZSPACE_OBJECT_MESHFIELD_H
#define ZSPACE_OBJECT_MESHFIELD_H

#pragma once

#include <zspace/zInterface/objects/zObjectMesh.h>
#include <zspace/zCore/field/zField2D.h>

#include <vector>
using namespace std;

namespace zSpace
{
	template<typename T> class zFnMeshField;
	class zItMeshScalarField;
	class zItMeshVectorField;

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjectMeshField
	*	\brief A template 2D field mesh object class.
	*	\tparam				T			- Type to work with zScalar(scalar field) and zVector(vector field).
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/
	
	template<typename T>
	class ZSPACE_API zObjectMeshField : public zObjectMesh
	{
		template<typename U> friend class zFnMeshField;
		friend class zItMeshScalarField;
		friend class zItMeshVectorField;

	private:
		//--------------------------
		//---- PRIVATE ATTRIBUTES
		//--------------------------

		/*! \brief field 2D */
		zField2D<T> field;

	public:

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjectMeshField();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjectMeshField();		

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------
			
	};

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \typedef zObjectMeshScalarField
	*	\brief A 2D scalar field mesh object.
	*
	*	\since version 0.0.2
	*/
	typedef zObjectMeshField<zScalar> zObjectMeshScalarField;

	/*! \typedef zObjectMeshVectorField
	*	\brief A 2D scalar field mesh object.
	*
	*	\since version 0.0.2
	*/
	typedef zObjectMeshField<zVector> zObjectMeshVectorField;

	/** @}*/

	/** @}*/

	
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

	/*! \typedef zObjectMeshScalarFieldArray
	*	\brief A vector of zObjectMeshScalarField.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectMeshScalarField> zObjectMeshScalarFieldArray;

	/*! \typedef zObjectMeshVectorFieldArray
	*	\brief A vector of zObjectMeshVectorField.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectMeshVectorField> zObjectMeshVectorFieldArray;

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}


#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/objects/zObjectMeshField.cpp>
#endif

#endif
