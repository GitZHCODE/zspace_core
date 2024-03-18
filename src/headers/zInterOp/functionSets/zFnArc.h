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

#ifndef ZSPACE_FN_ARC_H
#define ZSPACE_FN_ARC_H

#pragma once

#if defined (ZSPACE_RHINO_INTEROP)

#include "zInterface/functionsets/zFn.h"
#include "zInterop/objects/zObjArc.h"
#include "zInterOp/objects/zObjPlane.h"
#include "zInterOp/objects/zObjNurbsCurve.h"
#include "zInterop/functionsets/zFnNurbsCurve.h"

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

	/*! \class zFnArc
	*	\brief An arc function set.
	*	\since version 0.0.4
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zFnArc : protected zFn
	{	

	protected:
		//--------------------------
		//---- PROTECTED ATTRIBUTES
		//--------------------------
		/*!	\brief pointer to a arc object  */
		zObjArc *arcObj;

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
		zFnArc();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_arcObj			- input arcObj object.
		*	\since version 0.0.4
		*/
		zFnArc(zObjArc& _arcObj);


		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.4
		*/
		~zFnArc();

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

		/*! \brief This method creates a circle from the input plane containers.
		*
		*	\param		[in]	_plane		- container of zObjPlane.
		*	\param		[in]	_radius	    - container of radius.
		*	\param		[in]	_angle		- container of angle.
		*  	\return				bool        - boolean of success or not
		*	\since version 0.0.4
		*/
		bool create(zObjPlane& _plane, double _radius, double _angle = Z_TWO_PI);			

		/*! \brief This method creates a circle through three 3d points.
		*
		*	\param		[in]	p0		    - container of first point.
		*	\param		[in]	p1	        - container of second point.
		*	\param		[in]	p2		    - container of third point.
		*  	\return				bool        - boolean of success or not
		*	\since version 0.0.2
		*/ 
		bool createFromPoints(zPoint p0, zPoint p1, zPoint p2);

		/*! \brief This method creates a circle from two 3d points and a  tangent at the first point.
		*
		*	\param		[in]	p0		    - container of first point.
		*	\param		[in]	t1	        - container of tangent vector of first point.
		*	\param		[in]	p1		    - container of second point.
		*  	\return				bool        - boolean of success or not
		*	\since version 0.0.4
		*/
		bool createFromTangent(zPoint p0, zVector t1, zPoint p1);

		//bool IsInPlane(const ON_Plane&, double = ON_ZERO_TOLERANCE);
				
		//--------------------------
		//--- TOPOLOGY QUERY METHODS 
		//--------------------------
		
		/*! \brief This method returns the number of control vertices of the curve.
		*	\return				int         - number of control vertices.
		*	\since version 0.0.4
		*/
		int numControlVertices();	

		//--------------------------
		//--- COMPUTE METHODS 
		//--------------------------

		void intersect(zPlane &plane, zPoint &intersectionPt, double &t);		

		//--------------------------
		//--- SET METHODS 
		//--------------------------

		/*! \brief This method sets curve color to the input color.
		*
		*	\param		[in]	_col		 - input color.
		*	\since version 0.0.4
		*/
		void setArcDisplayColor(zColor _col);
	
		/*! \brief This method sets edge weight of the curve to the input weight.
		*
		*	\param		[in]	_wt		     - input weight.
		*	\since version 0.0.4
		*/
		void setArcDisplayWeight(double _wt);

		/*! \brief This method sets angle of arc to the input number.
		*
		*	\param		[in]	_angle		 - input angle between 0 and two PI.
		*	\since version 0.0.4
		*/
		void setAngle(double _angle);

		/*! \brief This method sets radius of arc to the input number.
		*
		*	\param		[in]	_radius		 - input radius.
		*	\since version 0.0.4
		*/
		void setRadius(double _radius);

		//--------------------------
		//--- COMPUTE METHODS 
		//--------------------------

		/*! \brief This method computes the display points on the arc.
		*
		*	\param		[in]	numPoints	 - number of display points required, 180 by default.
		*	\since version 0.0.4
		*/
		void computeArcPositions(int numPoints = 180);

		/*! \brief This method computes the control points of the arc.
		*
		*	\since version 0.0.4
		*/
		void computeControlPoints();

		/*! \brief This method checks if this arc is a circle.
		*
		*	\return			bool			 - boolean of isCircle or not
		*	\since version 0.0.4
		*/
		bool isCircle();		

		//--------------------------
		//--- GET METHODS 
		//--------------------------

		/*! \brief This method gets the angle of arc.
		*
		*	\return			double			 - angle of arc
		*	\since version 0.0.4
		*/
		double getAngle();


		/*! \brief This method gets the Radius of arc.
		*
		*	\return			double			 - Radius of arc
		*	\since version 0.0.4
		*/
		double getRadius();

		/*! \brief This method gets pointer to the internal curve positions container.
		*
		*	\return				zPoint*		 - pointer to internal vertex position container.
		*	\since version 0.0.4
		*/
		zPoint* getRawControlPoints();

		/*! \brief This method gets pointer to the internal curve positions container.
		*
		*	\param		[out]	numPoints	 - output number of points in container.
		*	\return				zPoint*		 - pointer to internal vertex position container.
		*	\since version 0.0.4
		*/
		zPoint* getRawArcPositions(int& numPoints);

		/*! \brief This method gets point at the specified parameter value.
		*
		*	\param		[in]	t			 - parameter value ( between 0 & 1).
		*	\return				zPoint		 - point at parameter value.
		*	\since version 0.0.4
		*/
		zPoint getPointAt(double t);

		/*! \brief This method gets tangent at the specified parameter value.
		*
		*	\param		[in]	t			 - parameter value ( between 0 & 1).
		*	\return				zVector		 - tangent at parameter value.
		*	\since version 0.0.4
		*/
		zVector getTangentAt(double t);

		/*! \brief This method gets tangent at the specified parameter value.
		*
		*	\param		[in]	t			 - parameter value between 0 & 1.
		*	\param		[out]	p			 - point at parameter value.
		*	\param		[out]	tan			 - tangent at parameter value.
		*	\since version 0.0.4
		*/
		void getPointTangentAt(double t, zPoint &p, zVector &tan);
						
		/*! \brief This method gets the center of the arc curve.
		*
		*	\return		zPoint				 - the center of the arc curve.
		*	\since version 0.0.4
		*/
		zPoint getCenter();

		/*! \brief This method gets the length of the curve.
		*
		*	\return		double				 - length of the arc curve .
		*	\since version 0.0.4
		*/
		double getLength();

		/*! \brief This method gets the domain of the curve.
		*
		*	\return		zDomainDouble		 - domain of the curve .
		*	\since version 0.0.4
		*/
		//zDomainDouble getDomain();

		/*! \brief This method gets the knot of the curve.
		*
		*	\return		zDoubleArray		 - container of knot vector of the curve .
		*	\since version 0.0.4
		*/
		//zDoubleArray getKnotVector();

		/*! \brief This method gets the degree of the curve.
		*
		*	\return		int					 - degree of the curve.
		*	\since version 0.0.4
		*/
		//int getDegree();
		
		/*! \brief This method gets the Raw OpenNurbs Arc object.
		*
		*	\return		ON_Circle	         - the Raw OpenNurbs Arc object.
		*	\since version 0.0.4
		*/
		ON_Circle* getRawON_Arc();

		
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

		//bool ON_NurbsCurve_GetLength(const ON_NurbsCurve& curve, double* length, double fractional_tolerance = 1.0e-8, const ON_Interval* sub_domain = NULL);
		
	};


}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterop/functionsets/zFnArc.cpp>
#endif

#endif
#endif