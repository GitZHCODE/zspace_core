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

#ifndef ZSPACE_OBJ_Arc_H
#define ZSPACE_OBJ_Arc_H

#pragma once

#include "zInterface/objects/zObj.h"
#include "zInterOp/include/zRhinoInclude.h"
#include "zInterOp/objects/zObjPlane.h"
#include "zInterOp/objects/zObjNurbsCurve.h"

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

	/*! \class zObjArc
	*	\brief The arc object class using OpenNURBS
	*	\details https://github.com/mcneel/opennurbs
	*	\since version 0.0.4
	*/

	/** @}*/
	
	/** @}*/

	class ZSPACE_API zObjArc : public zObj
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
		zObjPlane oPlane;
		
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
		zObjArc();

		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.4
		*/
		~zObjArc();

		//--------------------------
		//---- SET METHODS
		//--------------------------

		/*! \brief This method sets display arc, Control Points and Plane booleans.
		*
		*	\param		[in]	_displayArc		            - input display arc booelan.
		*	\param		[in]	_displayControlPoints		- input display Control Points booelan.
		*   \param		[in]	_displayPlane				- input display Plane booelan.
		*	\since version 0.0.4
		*/
		void setDisplayElements(bool _displayArc, bool _displayControlPoints, bool _displayPlane);


		/*! \brief This method sets display arc color.
		*
		*	\param		[in]	_arcDisplayColor		    - input display arc color.
		*	\since version 0.0.4
		*/
		void setArcDisplayColor(zColor _arcDisplayColor);

		/*! \brief This method sets display control Points Color.
		*
		*	\param		[in]	_controlPointsColor		    - input display control Points Color.
		*	\since version 0.0.4
		*/
		void setControlPointsColor(zColor _controlPointsColor);

		/*! \brief This method sets display control Poly Color.
		*
		*	\param		[in]	_controlPolyColor		    - input control Poly Color.
		*	\since version 0.0.4
		*/
		void setControlPolyColor(zColor _controlPolyColor);

		/*! \brief This method sets display control Points Weight.
		*
		*	\param		[in]	_controlPointsColor		    - input display control Points Weight.
		*	\since version 0.0.4
		*/
		void setControlPointsWeight(double _controlPointsWeight);

		/*! \brief This method sets display control Poly Weight.
		*
		*	\param		[in]	_controlPolyColor		    - input control Poly Weight.
		*	\since version 0.0.4
		*/
		void setControlPolyWeight(double _controlPolyWeight);

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
		void getRawPlane(zObjPlane _oPlane);


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
		*	\since version 0.0.4
		*/
		void appendToBuffer();

	protected:
		//--------------------------
		//---- PROTECTED DISPLAY METHODS
		//--------------------------

		/*! \brief This method displays the zObjArc.
		*
		*	\since version 0.0.4
		*/
		void drawArc();
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

	/*! \typedef zObjArcArray
	*	\brief A vector of zObjArc.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjArc> zObjArcArray;

	/*! \typedef zObjArcPointerArray
	*	\brief A vector of zObjArc pointers.
	*
	*	\since version 0.0.4
	*/
	typedef vector<zObjArc*>zObjArcPointerArray;

	/** @}*/
	/** @}*/
	/** @}*/
	/** @}*/
}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterOp/objects/zObjArc.cpp>
#endif

#endif