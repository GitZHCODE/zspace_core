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

#include <memory>
#include <type_traits>
#include <vector>
using namespace std;

namespace zSpace
{
	class zFieldObjectStorage;
	class zFnPointScalarField;
	class zFnPointVectorField;

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjectPointScalarField
	*	\brief A 3D scalar point field object class.
	*	\since version 0.0.2
	*/
	class ZSPACE_API zObjectPointScalarField : public zObjectPointCloud
	{
		friend class zFieldObjectStorage;
		friend class zFnPointScalarField;
		friend class zFnPointVectorField;

	private:
		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> fieldImpl;

	public:
		zObjectPointScalarField();
		zObjectPointScalarField(const zObjectPointScalarField& other);
		zObjectPointScalarField(zObjectPointScalarField&& other) noexcept;
		zObjectPointScalarField& operator=(const zObjectPointScalarField& other);
		zObjectPointScalarField& operator=(zObjectPointScalarField&& other) noexcept;
		~zObjectPointScalarField();
	};

	/*! \class zObjectPointVectorField
	*	\brief A 3D vector point field object class.
	*	\since version 0.0.2
	*/
	class ZSPACE_API zObjectPointVectorField : public zObjectPointCloud
	{
		friend class zFieldObjectStorage;
		friend class zFnPointScalarField;
		friend class zFnPointVectorField;

	private:
		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> fieldImpl;

	public:
		zObjectPointVectorField();
		zObjectPointVectorField(const zObjectPointVectorField& other);
		zObjectPointVectorField(zObjectPointVectorField&& other) noexcept;
		zObjectPointVectorField& operator=(const zObjectPointVectorField& other);
		zObjectPointVectorField& operator=(zObjectPointVectorField&& other) noexcept;
		~zObjectPointVectorField();
	};

	template<typename T>
	using zObjectPointField = std::conditional_t<std::is_same_v<T, zScalar>, zObjectPointScalarField, zObjectPointVectorField>;

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
