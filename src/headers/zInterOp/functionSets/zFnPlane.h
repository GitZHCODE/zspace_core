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

#if defined (ZSPACE_RHINO_INTEROP)

#include "zInterface/objects/zObj.h"
#include "zInterOp/include/zRhinoInclude.h"
#include "zInterface/functionsets/zFn.h"
#include "zInterop/objects/zObjPlane.h"

namespace zSpace
{
	/** \addtogroup zInterOp
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zFuntionSets
	*	\brief The function set classes of the library.
	*  @{
	*/

	/*! \class zFnPlane
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

		/*!	\brief pointer to a zObjPlane object  */
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
		*	\since version 0.0.4
		*/
		zFnPlane();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_planeObj			- input zObjPlane object.
		*	\since version 0.0.4
		*/
		zFnPlane(zObjPlane& _planeObj);


		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.4
		*/
		~zFnPlane();

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------

		zFnType getType() override;

		void from(string path, zFileType type, bool staticGeom = false) override;

		void from(json& j, bool staticGeom = false)override;

		void to(string path, zFileType type) override;

		void to(json& j) override;

#if defined ZSPACE_USD_INTEROP

		void from(UsdPrim& usd, bool staticGeom = false)override;

		void to(UsdPrim& usd)override;
#endif

		void getBounds(zPoint &minBB, zPoint &maxBB) override;

		void clear() override;

		//--------------------------
		//---- CREATE METHODS
		//--------------------------

		/*! \brief This method creates a graph from the input origin, xAxis, yAxis containers.
		*
		*	\param		[in]	origin		- container of input origin, zPoint(0,0,0) by default.
		*	\param		[in]	xAxis	    - container of input xAxis, zVector(1,0,0) by default.
		*	\param		[in]	yAxis		- container of input yAxis, zVector(0,1,0) by default.
		* 	\return		        bool		- boolean of success or not
		*	\since version 0.0.4
		*/
		bool create(zPoint origin = zPoint(0,0,0), zVector xAxis = zVector(1,0,0), zVector yAxis = zVector(0,1,0));

		/*! \brief This method creates a graph from the input origin, xAxis, yAxis containers.
		*
		*	\param		[in]	_plane		- container of input Eigen::Matrix4f.
		*   \return		        bool		- boolean of success or not
		*	\since version 0.0.4
		*/
		bool createFromMatrix(Eigen::Matrix4f & _plane);

		/*! \brief This method creates a graph from the input origin, xAxis, yAxis containers.
		*
		*	\param		[in]	origin		- container of input origin.
		* 	\param		[in]	normal		- container of input normal vector.
		* 	\param		[in]	yUp		    - container of input yUp vector, zVector(0,1,0) by default.
		*   \return		        bool		- boolean of success or not
		*	\since version 0.0.4
		*/
		bool createFromNormal(zPoint origin, zVector normal, zVector yUp = zVector(0,1,0));

		/*! \brief This method creates a graph from the input 3 points position.
		*
		*	\param		[in]	p0			- container of point 0, used as origin of plane.
		*	\param		[in]	p1	    	- container of point 1, used as xAxis direction.
		*	\param		[in]	p2			- container of point 2, used as yAxis direction.
		*   \return		        bool		- boolean of success or not
		*	\since version 0.0.4
		*/
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
		*	\param		[in]	xCol				- input color of xAxis, zColor(1.0, 0.0, 0.0, 1.0) by default.
		*   \param		[in]	yCol				- input color of yAxis, zColor(0.0, 1.0, 0.0, 1.0) by default.
		*   \param		[in]	zCol				- input color of zAxis, zColor(0.0, 0.0, 1.0, 1.0) by default.
		*	\since version 0.0.4
		*/
		void setDisplayColor(zColor xCol = zColor(1.0, 0.0, 0.0, 1.0), zColor yCol = zColor(0.0, 1.0, 0.0, 1.0), zColor zCol = zColor(0.0, 0.0, 1.0, 1.0));

	
		/*! \brief This method sets Display Weight of plane.
		*
		*	\param		[in]	_wt				     - input Display Weight of plane.
		*	\since version 0.0.4
		*/
		void setDisplayWeight(double _wt);

		/*! \brief This method sets origin position of the plane.
		*
		*	\param		[in]	origin				 - input origin position.
		*	\since version 0.0.4
		*/
		void setOrigin(zPoint origin);

		/*! \brief This method sets xAxis, yAxis and noral of the plane.
		*
		*	\param		[in]	xAxis				 - input xAxis vector.
		*   \param		[in]	yAxis				 - input yAxis vector.
		*   \param		[in]	normal				 - input normal vector.
		*	\since version 0.0.4
		*/
		void setAxis(zVector xAxis, zVector yAxis, zVector normal);

		//--------------------------
		//--- GET METHODS 
		//--------------------------

		/*! \brief This method gets the origin the plane.
		*
		*	\return		zPoint					     - origin of the plane.
		*	\since version 0.0.4
		*/
		zPoint getOrigin();

		/*! \brief This method gets the xAxis the plane.
		*
		*	\return		zPoint					     - xAxis of the plane.
		*	\since version 0.0.4
		*/
		zVector getXAxis();

		/*! \brief This method gets the yAxis the plane.
		*
		*	\return		zPoint					     - yAxis of the plane.
		*	\since version 0.0.4
		*/
		zVector getYAxis();

		/*! \brief This method gets the Normal the plane.
		*
		*	\return		zPoint					     - Normal of the plane.
		*	\since version 0.0.4
		*/
		zVector getNormal();
		
		/*! \brief This method gets the Raw OpenNurbs Plane object.
		*
		*	\return				ON_Plane			-  Raw OpenNurbs Plane object.
		*	\since version 0.0.4
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
#endif

