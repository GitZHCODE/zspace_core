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

#ifndef ZSPACE_OBJECT_GRAPH_H
#define ZSPACE_OBJECT_GRAPH_H

#pragma once

#include <zspace/zInterface/objects/zObject.h>
#include <zspace/export.h>

#include <memory>
#include <vector>
using namespace std;

namespace zSpace
{
	class zGraphObjectStorage;
	class zFnGraph;
	class zFnGraphDynamics;
	class zItGraphVertex;
	class zItGraphEdge;
	class zItGraphHalfEdge;
	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjectGraph
	*	\brief The graph object class.
	*	\since version 0.0.2
	*/

	/** @}*/
	
	/** @}*/

	class ZSPACE_API zObjectGraph : public zObject
	{
	private:
		class Impl;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		std::unique_ptr<Impl> impl;

		friend class zGraphObjectStorage;
		friend class zFnGraph;
		friend class zFnGraphDynamics;
		friend class zItGraphVertex;
		friend class zItGraphEdge;
		friend class zItGraphHalfEdge;

	protected:
		

	public:
		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjectGraph();
		zObjectGraph(const zObjectGraph& other);
		zObjectGraph(zObjectGraph&& other) noexcept;
		zObjectGraph& operator=(const zObjectGraph& other);
		zObjectGraph& operator=(zObjectGraph&& other) noexcept;

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjectGraph() override;

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

	/*! \typedef zObjectGraphArray
	*	\brief A vector of zObjectGraph.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectGraph> zObjectGraphArray;

	/*! \typedef zObjectGraphPointerArray
	*	\brief A vector of zObjectGraph pointers.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectGraph*>zObjectGraphPointerArray;

	/*! \brief Transitional keyword-first public graph object name. */

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/objects/zObjectGraph.cpp>
#endif

#endif
