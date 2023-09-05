// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2023 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//

#ifndef ZSPACE_OBJ_NURBSCURVE_H
#define ZSPACE_OBJ_NURBSCURVE_H

#pragma once

#include <headers/zInterface/objects/zObj.h>
#include <headers/zInterOp/include/zRhinoInclude.h>

#include <vector>
//using namespace std;

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

	/*! \class zObjNurbsCurve
	*	\brief The nurbs curve object class using OpenNURBS
	*	\details https://github.com/mcneel/opennurbs
	*	\since version 0.0.4
	*/

	/** @}*/
	
	/** @}*/

	class ZSPACE_API zObjNurbsCurve : public zObj
	{
	private:
		/*! \brief boolean for displaying the vertices */
		bool displayControlPoints;

		/*! \brief boolean for displaying the edges */
		bool displayCurve;

		/*!	\brief stores the start vertex ID in the VBO, when attached to the zBufferObject.	*/
		int VBO_ControlPointId;

		/*!	\brief stores the start edge ID in the VBO, when attached to the zBufferObject.	*/
		int VBO_CurvePointId;

		/*!	\brief stores the start vertex color ID in the VBO, when attache to the zBufferObject.	*/
		int VBO_CurveColorId;		
		

	protected:
		
		

	public:
		//--------------------------
		//---- PUBLIC ATTRIBUTES
		//--------------------------
		
		/*!	\brief container which stores positions of curve points for display.			*/
		zPointArray displayPositions;

		/*!	\brief stores color of the curve.			*/
		zColor displayColor;

		/*!	\brief stores weight of the curve.			*/
		double displayWeight;

		/*!	\brief container which stores positions of curve points.			*/
		zPointArray controlPoints;

		/*!	\brief container which stores weights of curve points.			*/
		zDoubleArray controlPointWeights;

		/*!	\brief stores degree of the curve.			*/
		int degree;

		/*!	\brief stores if the curve is perodic or not.			*/
		bool periodic;

		/*!	\brief OpenNURBS curve			*/
		ON_NurbsCurve curve;

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zObjNurbsCurve();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zObjNurbsCurve();

		//--------------------------
		//---- SET METHODS
		//--------------------------

		/*! \brief This method sets display vertices, edges and face booleans.
		*
		*	\param		[in]	_displayControlPoints		- input display controlPoints booelan.
		*	\param		[in]	_displayCurve				- input display curve booelan.
		*	\since version 0.0.2
		*/
		void setDisplayElements(bool _displayControlPoints, bool _displayCurve);


		//--------------------------
		//---- GET METHODS
		//--------------------------

		/*! \brief This method gets the vertex VBO Index .
		*
		*	\return			int				- vertex VBO Index.
		*	\since version 0.0.2
		*/
		int getVBO_ControlPointId();

		/*! \brief This method gets the edge VBO Index .
		*
		*	\return			int				- edge VBO Index.
		*	\since version 0.0.2
		*/
		int getVBO_CurvePointId();

		/*! \brief This method gets the vertex color VBO Index .
		*
		*	\return			int				- vertex color VBO Index.
		*	\since version 0.0.2
		*/
		int getVBO_CurveColorId();

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		void draw() override;
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
		void drawNurbsCurve();
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

	/*! \typedef zObjNurbsCurveArray
	*	\brief A vector of zObjNurbsCurve.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjNurbsCurve> zObjNurbsCurveArray;

	/*! \typedef zObjNurbsCurvePointerArray
	*	\brief A vector of zObjNurbsCurve pointers.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjNurbsCurve*>zObjNurbsCurvePointerArray;

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterOp/objects/zObjNurbsCurve.cpp>
#endif

#endif