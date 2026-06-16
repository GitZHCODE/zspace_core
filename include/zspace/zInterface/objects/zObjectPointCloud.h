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

#ifndef ZSPACE_OBJECT_POINTCLOUD_H
#define ZSPACE_OBJECT_POINTCLOUD_H

#pragma once

#include <zspace/zInterface/objects/zObject.h>
#include <zspace/export.h>

#include <memory>

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

	/*! \class zObjectPointCloud
	*	\brief The point cloud object class.
	*	\since version 0.0.2
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zObjectPointCloud :public zObject
	{
	private:
		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> impl;

		friend class zFnPointCloud;
		friend class zItPointCloudVertex;

	public:
		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------
		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjectPointCloud();
		zObjectPointCloud(const zObjectPointCloud& other);
		zObjectPointCloud(zObjectPointCloud&& other) noexcept;
		zObjectPointCloud& operator=(const zObjectPointCloud& other);
		zObjectPointCloud& operator=(zObjectPointCloud&& other) noexcept;

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjectPointCloud() override;

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

	/*! \typedef zObjectPointCloudArray
	*	\brief A vector of zObjectPointCloud.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectPointCloud> zObjectPointCloudArray;

	/*! \brief Transitional keyword-first public point-cloud object name. */

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/objects/zObjectPointCloud.cpp>
#endif

#endif
