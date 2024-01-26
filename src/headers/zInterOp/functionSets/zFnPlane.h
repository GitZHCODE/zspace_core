// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Ling Mao <Ling.Mao@zaha-hadid.com>
//


#ifndef ZSPACE_FN_PLANE_H
#define ZSPACE_FN_PLANE_H

#pragma once

#include "zInterface/functionsets/zFn.h"
#include "zInterop/objects/zObjPlane.h"

namespace zSpace
{
	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zFuntionSets
	*	\brief The function set classes of the library.
	*  @{
	*/

	/*! \class zFnNurbsCUrve
	*	\brief An arc function set.
	*	\since version 0.0.4
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zFnPlane : protected zFn
	{	

	protected:
		//--------------------------
		//---- PROTECTED ATTRIBUTES
		//--------------------------

		/*!	\brief pointer to a graph object  */
		zObjPlane *planeObj;	

	public:

		//--------------------------
		//---- PUBLIC ATTRIBUTES
		//--------------------------
		
		//--------------------------
		//---- CONSTRUCTOR
		//--------------------------

		/*! \brief Default constructor.
		*
		*	\since version 0.0.2
		*/
		zFnPlane();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_nurbsCurveObj			- input nurbscurve object.
		*	\since version 0.0.2
		*/
		zFnPlane(zObjPlane& _planeObj);


		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zFnPlane();

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------

		zFnType getType() override;

		void from(string path, zFileTpye type, bool staticGeom = false) override;

		void from(json& j, bool staticGeom = false)override;

		void to(string path, zFileTpye type) override;

		void to(json& j) override;

		void getBounds(zPoint &minBB, zPoint &maxBB) override;

		void clear() override;

		//--------------------------
		//---- CREATE METHODS
		//--------------------------

		/*! \brief This method creates a graph from the input containers.
		*
		*	\param		[in]	_positions		- container of type zPoint containing control point positions.
		*	\param		[in]	edgeConnects	- container of edge connections with vertex ids for each edge
		*	\param		[in]	staticGraph		- makes the graph fixed. Computes the static edge vertex positions if true.
		*	\since version 0.0.2
		*/
		bool create(zPoint origin = zPoint(0,0,0), zVector xAxis = zVector(1,0,0), zVector yAxis = zVector(0,1,0));

		bool createFromMatrix(Eigen::Matrix4f & _plane);

		bool createFromNormal(zPoint origin, zVector normal, zVector yUp = zVector(0,1,0));

		bool createFromPoints(zPoint p0, zPoint p1, zPoint p2);				

		//--------------------------
		//--- TOPOLOGY QUERY METHODS 
		//--------------------------
		
		//--------------------------
		//--- COMPUTE METHODS 
		//--------------------------

		void intersect();		

		//--------------------------
		//--- SET METHODS 
		//--------------------------

		/*! \brief This method sets curve color to the input color.
		*
		*	\param		[in]	_col				- input color.
		*	\since version 0.0.2
		*/
		void setDisplayColor(zColor xCol, zColor yCol, zColor zCol);

	
		/*! \brief This method sets edge weight of the curve to the input weight.
		*
		*	\param		[in]	_wt				- input weight.
		*	\since version 0.0.2
		*/
		void setDisplayWeight(double _wt);

		void setOrigin(zPoint origin);

		void setAxis(zVector xAxis, zVector yAxis, zVector normal);

		//--------------------------
		//--- GET METHODS 
		//--------------------------

		/*! \brief This method gets the center the curve.
		*
		*	\return		zPoint					- center .
		*	\since version 0.0.4
		*/
		zPoint getOrigin();

		zVector getXAxis();

		zVector getYAxis();

		zVector getNormal();
		
		/*! \brief This method computes the lengths of all the half edges of a the graph.
		*
		*	\param		[out]	halfEdgeLengths				- vector of halfedge lengths.
		*	\return				double						- total edge lengths.
		*	\since version 0.0.2
		*/
		ON_Plane* getRawON_Plane();

		
		//--------------------------
		//---- TRANSFORM METHODS OVERRIDES
		//--------------------------

		void setTransform(zTransform &inTransform, bool decompose = true, bool updatePositions = true) override;

		void setScale(zFloat4 &scale) override;

		void setRotation(zFloat4 &rotation, bool appendRotations = false) override;

		void setTranslation(zVector &translation, bool appendTranslations = false) override;

		void setPivot(zVector &pivot) override;

		void getTransform(zTransform &transform) override;
		
	

	protected:

		//--------------------------
		//---- PROTECTED OVERRIDE METHODS
		//--------------------------	
		void transformObject(zTransform& transform) override;

	private:

		//--------------------------
		//---- PRIVATE METHODS
		//--------------------------

	};


}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterop/functionsets/zFnPlane.cpp>
#endif

#endif

