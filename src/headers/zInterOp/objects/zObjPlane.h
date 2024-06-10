// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2023 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Ling Mao <Ling.Mao@zaha-hadid.com>
//

#ifndef ZSPACE_OBJ_PLANE_H
#define ZSPACE_OBJ_PLANE_H

#if defined (ZSPACE_RHINO_INTEROP)

#pragma once

#include "zInterface/objects/zObj.h"
#include "zInterOp/include/zRhinoInclude.h"

#include <vector>
//using namespace std;

namespace zSpace
{
	/** \addtogroup zInterOp
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zObjects
	*	\brief The object classes of the library.
	*  @{
	*/

	/*! \class zObjPlane
	*	\brief The plane object class using OpenNURBS
	*	\details https://github.com/mcneel/opennurbs
	*	\since version 0.0.4
	*/

	/** @}*/
	
	/** @}*/

	class ZSPACE_API zObjPlane : public zObj
	{
	private:				

	protected:			

	public:
		//--------------------------
		//---- PUBLIC ATTRIBUTES
		//--------------------------

		/*! \brief boolean for displaying the Axis. */
		bool displayAxis = true;

		/*! \brief boolean for displaying the rectangle. */
		bool displayRectangle = false;

		/*! \brief stores displayAxisScale. */
		double displayAxisScale = 1;

		/*! \brief stores displayRectangleScale. */
		double displayRectangleScale = 1;	

		/*!	\brief stores origin of the plane. */
		zPoint origin = zPoint(0, 0, 0);

		/*!	\brief stores xAxis of the plane. */
		zVector xAxis = zVector(1, 0, 0);

		/*!	\brief stores yAxis of the plane. */
		zVector yAxis = zVector(0, 1, 0);

		/*!	\brief stores normal of the plane. */
		zVector normal = zVector(0, 0, 1);

		/*!	\brief stores color of the axis. */
		zColorArray displayColor;

		/*!	\brief stores weight of the axis. */
		double displayWeight = 3;

		/*!	\brief OpenNURBS plane object. */
		ON_Plane on_plane;

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.4
		*/
		zObjPlane();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.4
		*/
		~zObjPlane();

		//--------------------------
		//---- SET METHODS
		//--------------------------

		/*! \brief This method sets display Axis and Rectangle plane booleans and scales.
		*
		*	\param		[in]	_displayAxis		        - input display Axis booelan.
		*	\param		[in]	_displayRectangle			- input display Rectangle booelan.
		*   \param		[in]	_displayAxisScale		    - input display Axis Scale.
		*	\param		[in]	_displayRectangleScale		- input display Rectangle Scale.
		*	\since version 0.0.4
		*/
		void setDisplayElements(bool _displayAxis, bool _displayRectangle, double _displayAxisScale, double _displayRectangleScale);

		/*! \brief This method sets display color array.
		*
		*	\param		[in]	_displayColorArray		    - input display Axis booelan.
		*   \since version 0.0.4
		*/
		void setDisplayColor(zColorArray _displayColorArray);

		//--------------------------
		//---- GET METHODS
		//--------------------------

		/*! \brief This method gets a 4x4 Matrix of the plane.
		*	\return				zPlane				- a 4x4 Matrix of the plane.
		*	\since version 0.0.4
		*/
		zPlane getPlaneMatrix();
		
		/*! \brief This method gets a Double Array of the plane.
		*	\return				zDoubleArray		- a Double Array of the plane.
		*	\since version 0.0.4
		*/
		zDoubleArray getDoubleArray();


		/*! \brief This method stores Raw Origin of the plane.
		*
		*	\param		[out]	_origin	        - stores Raw Origin of the plane.
		*	\since version 0.0.4
		*/
		void getRawOrigin(zPoint &_origin);

		/*! \brief This method stores Raw xAxis vector of the plane.
		*
		*	\param		[out]	_xAxis      	- stores Raw xAxis vector of the plane.
		*	\since version 0.0.4
		*/
		void getRawXAxis(zVector& _xAxis);

		/*! \brief This method stores Raw yAxis vector of the plane.
		*
		*	\param		[out]	_yAxis	        - stores Raw yAxis vector of the plane.
		*	\since version 0.0.4
		*/
		void getRawYAxis(zVector& _yAxis);

		/*! \brief This method stores Raw Normal vector of the plane.
		*
		*	\param		[out]	_Normal	         - stores Raw Normal vector of the plane.
		*	\since version 0.0.4
		*/
		void getRawNormal(zVector& _Normal);

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------

#if defined (ZSPACE_VIEWER) || defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		void draw() override;
#endif

		void getBounds(zPoint &minBB, zPoint &maxBB) override;

#ifndef ZSPACE_VIEWER
		//--------------------------
		//---- DISPLAY BUFFER METHODS
		//--------------------------

		/*! \brief This method appends zObjPlane to the buffer.
		*
		*	\since version 0.0.4
		*/
		void appendToBuffer();

	protected:
		//--------------------------
		//---- PROTECTED DISPLAY METHODS
		//--------------------------

		/*! \brief This method displays the zObjPlane.
		*
		*	\since version 0.0.4
		*/
		void drawPlane();
#endif
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

	/*! \typedef zObjPlaneArray
	*	\brief A vector of zObjPlane.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjPlane> zObjPlaneArray;

	/*! \typedef zObjPlanePointerArray
	*	\brief A vector of zObjPlane pointers.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjPlane*>zObjPlanePointerArray;

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterOp/objects/zObjPlane.cpp>
#endif

#endif
#endif