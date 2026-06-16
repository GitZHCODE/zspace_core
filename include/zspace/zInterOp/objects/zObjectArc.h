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

#ifndef ZSPACE_OBJECT_ARC_H
#define ZSPACE_OBJECT_ARC_H

#pragma once

#include<zspace/zInterface/objects/zObject.h>
#include<zspace/zInterOp/sdk/zRhinoInclude.h>
#include<zspace/zInterOp/objects/zObjectPlane.h>
#include<zspace/zInterOp/objects/zObjectNurbsCurve.h>

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

	/*! \class zObjectArc
	*	\brief The arc object class using OpenNURBS
	*	\details https://github.com/mcneel/opennurbs
	*	\since version 0.0.4
	*/

	/** @}*/
	
	/** @}*/

	class ZSPACE_INTEROP zObjectArc : public zObject
	{
	private:

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

		/*!	\brief container which stores radius of the arc.	 */
		double radius;

		/*!	\brief container which stores Angle Radians of the arc. */
		double angle = Z_TWO_PI;

		/*!	\brief container which stores boolean of isCircle. */
		bool isCircle = true;

		/*!	\brief container which stores positions of arc plane. */
		zObjectPlane oPlane;
		
		/*! \brief boolean for displaying the plane */
		bool displayPlane = false;
		
		/*!	\brief container which stores positions of arc points. */
		zPointArray controlPoints;

		/*! \brief boolean for displaying the vertices. */
		bool displayControlPoints = false;
		
		/*!	\brief stores color of the arc. */
		zColor controlPointsColor = zColor(0, 0, 0, 0);

		/*!	\brief stores color of the arc. */
		zColor controlPolyColor = zColor(0, 0, 1, 0);

		/*!	\brief container which stores weights of arc points.	 */
		double controlPointsWeight = 5;

		/*!	\brief container which stores weights of arc points.	 */
		double controlPolyWeight = 1;

		/*! \brief boolean for displaying the curve */
		bool displayArc = true;

		/*!	\brief container which stores positions of arc for display.			*/
		zPointArray arcPositions;

		/*!	\brief stores display color of the arc.	 */
		zColor arcDisplayColor = zColor(1, 0, 0, 0);

		/*!	\brief stores weight of the arc. */
		double arcDisplayWeight = 3;

		/*!	\brief OpenNURBS arc object	*/
		ON_Arc on_arc;

		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.4
		*/
		zObjectArc();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.4
		*/
		~zObjectArc();

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

		/*! \brief This method gets Raw radius vector of the arc.
		*
		*	\param		[out]	_Normal	         - stores Raw radius vector of the arc.
		*	\since version 0.0.4
		*/
		void getRawRadius(double _radius);

		/*! \brief This method gets Raw Angle in radian of the arc.
		*
		*	\param		[out]	_Normal	         - stores Raw Angle in radian of the arc.
		*	\since version 0.0.4
		*/
		void getRawAngleRadian(double _angle);

		/*! \brief This method gets Raw plane of the arc.
		*
		*	\param		[out]	_Normal	         - stores Raw plane of the arc.
		*	\since version 0.0.4
		*/
		void getRawPlane(zObjectPlane _oPlane);
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

	/*! \typedef zObjectArcArray
	*	\brief A vector of zObjectArc.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectArc> zObjectArcArray;

	/*! \typedef zObjectArcPointerArray
	*	\brief A vector of zObjectArc pointers.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjectArc*>zObjectArcPointerArray;

	/*! \brief Keyword-first public arc object name. */

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterOp/objects/zObjectArc.cpp>
#endif

#endif
