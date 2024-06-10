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

#if defined (ZSPACE_RHINO_INTEROP)

#include "zInterface/objects/zObj.h"
#include "zInterOp/include/zRhinoInclude.h"

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

#ifndef ZSPACE_VIEWER
		/*!	\brief stores the start vertex ID in the VBO, when attached to the zBufferObject.	*/
		int VBO_ControlPointId;

		/*!	\brief stores the start edge ID in the VBO, when attached to the zBufferObject.	*/
		int VBO_CurvePointId;

		/*!	\brief stores the start vertex color ID in the VBO, when attache to the zBufferObject.	*/
		int VBO_CurveColorId;
#endif

	protected:
		
		/*!	\brief number of display positions.			*/
		int numDisplayPositions;

		/*!	\brief container which stores positions of curve points for display.			*/
		zPointArray displayPositions;

		/*!	\brief container which stores positions of curve control points.			*/
		zPointArray displayControlPointPositions;

		/*!	\brief stores color of the curve.			*/
		zColor displayColor;

		/*!	\brief stores weight of the curve.			*/
		double displayWeight;

		/*!	\brief stores dimension of the curve.			*/
		int degree;

		/*!	\brief stores periodic of the curve.			*/
		bool periodic;


	public:
		//--------------------------
		//---- PUBLIC ATTRIBUTES
		//--------------------------
		
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

		/*! \brief This method sets display vertex positions.
		*
		*	\param		[in]	_positions		- input display on curve positions.
		*	\since version 0.0.2
		*/
		void setDisplayPositions(zPointArray& _positions);

		/*! \brief This method sets curve degree.
		*
		*	\param		[in]	_degree		- input curve degree.
		*	\since version 0.0.2
		*/
		void setDegree(int _degree);

		/*! \brief This method sets curve periodic.
		*
		*	\param		[in]	_degree		- input curve periodic.
		*	\since version 0.0.2
		*/
		void setPeriodic(bool _periodic);

		/*! \brief This method sets curve display color.
		*
		*	\param		[in]	_displayColor		- input display color.
		*	\since version 0.0.2
		*/
		void setDisplayColor(zColor _displayColor);

		/*! \brief This method sets curve display weight.
		*
		*	\param		[in]	_displayColor		- input display weight.
		*	\since version 0.0.2
		*/
		void setDisplayWeight(double _displayWeight);

		/*! \brief This method sets curve control points.
		*
		*	\param		[in]	_controlPoints		- input curve control points.
		*	\since version 0.0.2
		*/
		void setDisplayControlPoints(zPointArray& _displayControlPoints);

		/*! \brief This method sets curve control point weights.
		*
		*	\param		[in]	_controlPointWeights		- input curve control point weights.
		*	\since version 0.0.2
		*/
		void setControlPointWeights(zDoubleArray& _controlPointWeights);

		//--------------------------
		//---- GET METHODS
		//--------------------------

		/*! \brief This method gives read/write access to individual control points in the zObjNurbsCurve storage
		*	\details  Used for updating CV positions
		*
		*	\param		[in]	index		- index of contol point in displayContolPointPositions array.
		*	\return		zPoint&				- reference to the specific zPoint.
		*	\since version 0.0.x
		*/
		zPoint& GetControlPoint(int index);

		const zPointArray& GetDisplayPositions() const { return displayPositions; }
		const zPointArray& GetControlPositions() const { return displayControlPointPositions; }
		const zColor& GetCurveColor() const { return displayColor; }

		/*! \brief This method gets display vertex positions.
		*
		*	\param		[out]	_positions		- output display on curve positions.
		*	\since version 0.0.2
		*/
		void getDisplayPositions(zPointArray& _positions);

		/*! \brief This method gets number of display vertex positions.
		*
		*	\return			int				- num display positions.
		*	\since version 0.0.2
		*/
		int getNumDisplayPositions();

		/*! \brief This method gets display curve boolean.
		*
		*	\return			bool				- display curve.
		*	\since version 0.0.4
		*/
		inline bool& getDisplayCurve() { return displayCurve; }

		/*! \brief This method gets display control points boolean.
		*
		*	\return			bool				- num display control points.
		*	\since version 0.0.4
		*/
		inline bool& getDisplayControlPoints() { return displayControlPoints; }

#ifndef ZSPACE_VIEWER
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
#endif

		/*! \brief This method gets curve dimension.
		*
		*	\return			int				- curve degree.
		*	\since version 0.0.2
		*/
		int getDegree();

		/*! \brief This method gets curve periodic.
		*
		*	\return			bool				- is periodic or not.
		*	\since version 0.0.2
		*/
		bool isPeriodic();

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
#endif