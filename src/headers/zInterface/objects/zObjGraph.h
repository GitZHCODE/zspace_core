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

#ifndef ZSPACE_OBJ_GRAPH_H
#define ZSPACE_OBJ_GRAPH_H

#pragma once

#include "zInterface/objects/zObj.h"
#include "zCore/geometry/zGraph.h"

#include <vector>
using namespace std;

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

	/*! \class zObjGraph
	*	\brief The graph object class.
	*	\since version 0.0.2
	*/

	/** @}*/
	
	/** @}*/

	class ZSPACE_API zObjGraph : public zObj
	{
	private:
		/*! \brief boolean for displaying the vertices */
		bool displayVertices;

		/*! \brief boolean for displaying the edges */
		bool displayEdges;

		/*! \brief boolean for displaying the edges */
		bool displayVertexIds = false;

		/*! \brief boolean for displaying the edges */
		bool displayEdgeIds = false;

		/*! \brief container for storing edge centers */
		zPointArray edgeCenters;

	protected:
		

	public:
		//--------------------------
		//---- PUBLIC ATTRIBUTES
		//--------------------------
		/*! \brief graph */
		zGraph graph;
		

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjGraph();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjGraph();

		//--------------------------
		//---- SET METHODS
		//--------------------------

		/*! \brief This method sets display vertices, edges and face booleans.
		*
		*	\param		[in]	_showVerts				- input display vertices booelan.
		*	\param		[in]	_showEdges				- input display edges booelan.
		*	\since version 0.0.2
		*/
		void setDisplayElements(bool _displayVertices, bool _displayEdges);

		/*! \brief This method sets display vertices, edges and face booleans.
		*
		*	\param		[in]	_showVertIds				- input display vertex Ids booelan.
		*	\param		[in]	_showEdgeIds				- input display edge Ids booelan.
		*	\since version 0.0.4
		*/
		void setDisplayElementIds(bool _displayVertexIds, bool _displayEdgeIds);

		/*! \brief This method sets display vertices boolean.
		*
		*	\param		[in]	_showVerts				- input display vertices booelan.
		*	\since version 0.0.2
		*/
		void setDisplayVertices(bool _displayVertices);


		/*! \brief This method sets display edges boolean.
		*
		*	\param		[in]	_showEdges				- input display edges booelan.
		*	\since version 0.0.2
		*/
		void setDisplayEdges(bool _displayEdges);

		/*! \brief This method sets edge centers container.
		*
		*	\param		[in]	_edgeCenters				- input edge center conatiner.
		*	\since version 0.0.4
		*/
		void setEdgeCenters(zPointArray &_edgeCenters);

		//--------------------------
		//---- GET METHODS
		//--------------------------

		/*! \brief This method gets display vertices boolean.
		*
		*	\return						bool				- input display vertices boolean.
		*	\since version 0.0.4
		*/
		inline bool& getDisplayVertices() { return displayVertices; }

		/*! \brief This method gets display edges boolean.
		*
		*	\return						bool				- input display edges boolean.
		*	\since version 0.0.4
		*/
		inline bool& getDisplayEdges() { return displayEdges; }

		/*! \brief This method gets display vertex id boolean.
		*
		*	\return						bool				- input display vertex id boolean.
		*	\since version 0.0.4
		*/
		inline bool& getDisplayVertexIds() { return displayVertexIds; }

		/*! \brief This method gets display edge id boolean.
		*
		*	\return						bool				- input display edge id boolean.
		*	\since version 0.0.4
		*/
		inline bool& getDisplayEdgeIds() { return displayEdgeIds; }

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		//void draw() override;
#endif

		void getBounds(zPoint &minBB, zPoint &maxBB) override;

		//--------------------------
		//---- DISPLAY BUFFER METHODS
		//--------------------------

		/*! \brief This method appends graph to the buffer.
		*
		*	\since version 0.0.1
		*/
		void appendToBuffer();

	protected:
		//--------------------------
		//---- PROTECTED DISPLAY METHODS
		//--------------------------

		/*! \brief This method displays the zGraph.
		*
		*	\since version 0.0.2
		*/
		void drawGraph();
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

	/*! \typedef zObjGraphArray
	*	\brief A vector of zObjGraph.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjGraph> zObjGraphArray;

	/*! \typedef zObjGraphPointerArray
	*	\brief A vector of zObjGraph pointers.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjGraph*>zObjGraphPointerArray;

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterface/objects/zObjGraph.cpp>
#endif

#endif