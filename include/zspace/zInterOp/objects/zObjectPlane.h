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

#ifndef ZSPACE_OBJECT_PLANE_H
#define ZSPACE_OBJECT_PLANE_H

#pragma once

#include <zspace/zInterface/objects/zObject.h>
#include <zspace/zInterOp/sdk/zRhinoInclude.h>

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

	/*! \class zObjectPlane
	*	\brief The plane object class using OpenNURBS
	*	\details https://github.com/mcneel/opennurbs
	*	\since version 0.0.4
	*/

	/** @}*/
	
	/** @}*/

	class ZSPACE_INTEROP zObjectPlane : public zObject
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
		zObjectPlane();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.4
		*/
		~zObjectPlane();

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

	/*! \typedef zObjectPlaneArray
	*	\brief A vector of zObjectPlane.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectPlane> zObjectPlaneArray;

	/*! \typedef zObjectPlanePointerArray
	*	\brief A vector of zObjectPlane pointers.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectPlane*>zObjectPlanePointerArray;

	/*! \brief Keyword-first public plane object name. */

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterOp/objects/zObjectPlane.cpp>
#endif

#endif
