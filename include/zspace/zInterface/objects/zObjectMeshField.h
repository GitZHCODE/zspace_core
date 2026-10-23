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

#include <memory>
#include <type_traits>
#include <vector>
using namespace std;

namespace zSpace
{
	class zFieldObjectStorage;
	class zFnMeshScalarField;
	class zFnMeshVectorField;

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjectMeshScalarField
	*	\brief A 2D scalar field mesh object class.
	*	\since version 0.0.2
	*/
	class ZSPACE_API zObjectMeshScalarField : public zObjectMesh
	{
		friend class zFieldObjectStorage;
		friend class zFnMeshScalarField;
		friend class zFnMeshVectorField;

	private:
		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> fieldImpl;

	public:
		zObjectMeshScalarField();
		zObjectMeshScalarField(const zObjectMeshScalarField& other);
		zObjectMeshScalarField(zObjectMeshScalarField&& other) noexcept;
		zObjectMeshScalarField& operator=(const zObjectMeshScalarField& other);
		zObjectMeshScalarField& operator=(zObjectMeshScalarField&& other) noexcept;
		~zObjectMeshScalarField();
	};

	/*! \class zObjectMeshVectorField
	*	\brief A 2D vector field mesh object class.
	*	\since version 0.0.2
	*/
	class ZSPACE_API zObjectMeshVectorField : public zObjectMesh
	{
		friend class zFieldObjectStorage;
		friend class zFnMeshScalarField;
		friend class zFnMeshVectorField;

	private:
		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> fieldImpl;

	public:
		zObjectMeshVectorField();
		zObjectMeshVectorField(const zObjectMeshVectorField& other);
		zObjectMeshVectorField(zObjectMeshVectorField&& other) noexcept;
		zObjectMeshVectorField& operator=(const zObjectMeshVectorField& other);
		zObjectMeshVectorField& operator=(zObjectMeshVectorField&& other) noexcept;
		~zObjectMeshVectorField();
	};

	template<typename T>
	using zObjectMeshField = std::conditional_t<std::is_same_v<T, zScalar>, zObjectMeshScalarField, zObjectMeshVectorField>;

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
