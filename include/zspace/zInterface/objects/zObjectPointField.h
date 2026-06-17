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

#ifndef ZSPACE_OBJECT_POINTFIELD_H
#define ZSPACE_OBJECT_POINTFIELD_H

#pragma once

#include <zspace/zInterface/objects/zObjectPointCloud.h>
#include <zspace/zCore/field/zField3D.h>

#include <vector>
using namespace std;

namespace zSpace
{
	template<typename T> class zFnPointField;
	class zItPointScalarField;
	class zItPointVectorField;

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjectPointField
	*	\brief A template 3D point field object class.
	*	\tparam				T			- Type to work with zScalar(scalar field) and zVector(vector field).
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/
	
	template<typename T>
	class ZSPACE_API zObjectPointField : public zObjectPointCloud
	{
		template<typename U> friend class zFnPointField;
		friend class zItPointScalarField;
		friend class zItPointVectorField;

	private:
		//--------------------------
		//---- PRIVATE ATTRIBUTES
		//--------------------------

		/*! \brief field 2D */
		zField3D<T> field;

	public:

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjectPointField();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjectPointField();
	
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

	/*! \typedef zObjectPointScalarField
	*	\brief A 2D scalar field mesh object.
	*
	*	\since version 0.0.2
	*/
	typedef zObjectPointField<zScalar> zObjectPointScalarField;

	/*! \typedef zObjectPointVectorField
	*	\brief A 2D scalar field mesh object.
	*
	*	\since version 0.0.2
	*/
	typedef zObjectPointField<zVector> zObjectPointVectorField;

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

	/*! \typedef zObjectPointScalarFieldArray
	*	\brief A vector of zObjectPointScalarField.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectPointScalarField> zObjectPointScalarFieldArray;

	/*! \typedef zObjectPointVectorFieldArray
	*	\brief A vector of zObjectPointVectorField.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectPointVectorField> zObjectPointVectorFieldArray;

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/

}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/objects/zObjectPointField.cpp>
#endif

#endif

