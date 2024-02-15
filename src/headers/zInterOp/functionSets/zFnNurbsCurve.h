// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//

#ifndef ZSPACE_FN_NURBSCURVE_H
#define ZSPACE_FN_NURBSCURVE_H

#pragma once

#include "zInterface/functionsets/zFn.h"
#include "zInterop/objects/zObjNurbsCurve.h"

#include "zInterface/objects/zObjGraph.h"
#include "zInterface/functionsets/zFnGraph.h"

#include "zInterOp/objects/zObjArc.h"
#include "zInterop/functionsets/zFnArc.h"
#include "zInterOp/objects/zObjPlane.h"
#include "zInterop/functionsets/zFnPlane.h"


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
	*	\brief A nurbs curve function set.
	*	\since version 0.0.4
	*/

	/** @}*/

	/** @}*/

	class ZSPACE_API zFnNurbsCurve : protected zFn
	{	

	protected:
		//--------------------------
		//---- PROTECTED ATTRIBUTES
		//--------------------------

		/*!	\brief pointer to a graph object  */
		zObjNurbsCurve *nurbsCurveObj;	


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
		zFnNurbsCurve();

		/*! \brief Overloaded constructor.
		*
		*	\param		[in]	_nurbsCurveObj			- input nurbscurve object.
		*	\since version 0.0.2
		*/
		zFnNurbsCurve(zObjNurbsCurve&_nurbsCurveObj);


		//--------------------------
		//---- DESTRUCTOR
		//--------------------------

		/*! \brief Default destructor.
		*
		*	\since version 0.0.2
		*/
		~zFnNurbsCurve();

		//--------------------------
		//---- OVERRIDE METHODS
		//--------------------------

		zFnType getType() override;

		void from(string path, zFileTpye type, bool staticGeom = false) override;

		void from(json& j, bool staticGeom = false)override;

		void to(string path, zFileTpye type) override;

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

		/*! \brief This method creates a nurbs curve from the input control points.
		*
		*	\param		[in]	positions			- container of type zPoint containing control point positions.
		*	\param		[in]	degree				- desired curve degree.
		*	\param		[in]	periodic			- desired curve periodic.
		*	\param		[in]	interpolate			- if true the method will create nurbs curve by interpolating the input positions.
		*	\param		[in]	displayNumPoints	- approxiamte number of points for displaying the curve.
		*	\since version 0.0.2
		*/
		void create(zPointArray& positions, int degree,  bool periodic, bool interpolate, int displayNumPoints = 20);

		/*! \brief This method creates a nurbs curve from the input obj graph.
		*
		*	\param		[in]	in_graph			- input obj graph.
		*	\param		[in]	sampleDist			- sample distance used to re sample the obj graph. skip resampling if sampleDist = 0
		*	\param		[in]	degree				- desired curve degree.
		*	\param		[in]	periodic			- desired curve periodic.
		*	\param		[in]	interpolate			- if true the method will create nurbs curve by interpolating the input positions.
		*	\param		[in]	displayNumPoints	- approxiamte number of points for displaying the curve.
		*	\warning	only works on a continuous polyline graph.
		*	\since version 0.0.2
		*/
		void create(zObjGraph& in_graph, double sampleDist, int degree, bool periodic, bool interpolate, int displayNumPoints);

		void create(zObjArc& o_arc, int cvNum, int displayNumPoints);

		//--------------------------
		//--- TOPOLOGY QUERY METHODS 
		//--------------------------
		
		/*! \brief This method returns the number of control vertices of the curve.
		*	\return				number of control vertices.
		*	\since version 0.0.4
		*/
		int numControlVertices();

		//--------------------------
		//--- COMPUTE METHODS
		//--------------------------

		/*! \brief This method computes the closest point on curve to the input test point.
		*
		*	\param		[in]	testPoint		- input point to test.
		*	\param		[out]	closestPoint	- output point on curve.
		*	\param		[out]	t				- output curve parameter of closest point.
		*	\since version 0.0.2
		*/
		void closestPoint(zPoint& testPoint, zPoint& closestPoint, double& t);

		/*! \brief This method computes the closest point between two curves .
		*
		*	\param		[in]	curveB			- input point to test.
		*	\param		[out]	closestPoint_A	- output point on curve A.
		*	\param		[out]	closestPoint_B	- output point on curve B.
		* 	\param		[out]	t_A				- output curve parameter on curve A.
		*	\param		[out]	t_B				- output curve parameter on curve B.
		*	\since version 0.0.2
		*/
		void closestPoint(zObjNurbsCurve& curveB, zPoint& closestPoint_A, zPoint& closestPoint_B, double& t_A, double& t_B);

		/*! \brief This method divides the curve by input segment count.
		*
		*	\param		[in]	numSegments		- input desired number of curve divisions.
		*	\param		[out]	positions		- output points on curve.
		*	\param		[out]	tParams			- output parameters at divisions.
		*	\since version 0.0.2
		*/
		void divideByCount(int numSegments, zPointArray& positions, zDoubleArray& tParams);

		/*! \brief This method divides the curve by input length.
		*
		*	\param		[in]	length			- input desired length of curve divisions.
		*	\param		[out]	positions		- output points on curve.
		*	\param		[out]	tParams			- output parameters at divisions.
		*	\since version 0.0.2
		*/
		void divideByLength(double length, zPointArray& positions, zDoubleArray& tParams);

		/*! \brief This method computes the sub curve from the input domain d0 to d1
		*
		*	\param		[in]	t0					- input domain start for subtraction.
		*	\param		[in]	t1					- input domain end for subtraction.
		*	\param		[in]	normalise			- normalise sub curve to (0,1) if true.
		*	\param		[out]	out_SubCurve		- output sub curve.
		*	\param		[in]	displayNumPoints	- output sub curve display num points.
		*	\since version 0.0.2
		*/
		void computeSubCurve(double t0, double t1, bool normalise, zObjNurbsCurve& out_SubCurve, int displayNumPoints = 100);

		/*! \brief This method reverses the curve direction.
		*
		*	\since version 0.0.2
		*/
		void reverse();

		//--------------------------
		//--- INTERSECT METHODS 
		//--------------------------

		/*! \brief This method computes the intersections to the input plane.
		*
		*	\param		[in]	plane			- input plane to intersect.
		*	\param		[out]	intersectionPts	- output intersection points if valid.
		*	\param		[out]	tParams			- output on curve parameters where intersections happened.
		*	\since version 0.0.2
		*/
		void intersect(zPlane &plane, zPointArray &intersectionPts, zDoubleArray & tParams);
		
		/*! \brief This method computes the intersections to the input curve.
		*
		*	\param		[in]	curveB				- input curve to intersect.
		*	\param		[out]	intersectionPts_A	- output intersection points on this curve if valid.
		*	\param		[out]	tParams_A			- output on curve parameters on this curve where intersections happened.
		*	\param		[out]	intersectionPts_B	- output intersection points on curveB if valid.
		*	\param		[out]	tParams_B			- output on curve parameters on curveB where intersections happened.
		*	\since version 0.0.2
		*/
		void intersect(zObjNurbsCurve& curveB, zPointArray& intersectionPts_A, zPointArray& intersectionPts_B, zDoubleArray& tParams_A, zDoubleArray& tParams_B);

		/*! \brief This method computes the intersection on the curve itself.
		*
		*	\param		[out]	intersectionPts	- output intersection points if valid.
		*	\param		[out]	tParams			- output on curve parameters where intersections happened.
		*	\since version 0.0.2
		*/
		void intersectSelf(zPointArray& intersectionPts, zDoubleArray& tParams);


		//--------------------------
		//--- SET METHODS 
		//--------------------------

		/*! \brief This method sets degree of the curve.
		*
		*	\param		[in]	_degree				- input degree.
		*	\since version 0.0.2
		*/
		void setDegree(int _degree, int _displayNumPoints);

		/*! \brief This method reparameterise the curve with new domain (t0, t1).
		*	\param		[in]	t0			- input domain start.
		*	\param		[in]	t1			- input domain end.
		*	\since version 0.0.2
		*/
		void setDomain(double t0 = 0.0, double t1 = 1.0);

		/*! \brief This method sets curve color to the input color.
		*
		*	\param		[in]	_col				- input color.
		*	\since version 0.0.2
		*/
		void setDisplayColor(zColor _col);

	
		/*! \brief This method sets edge weight of the curve to the input weight.
		*
		*	\param		[in]	_wt				- input weight.
		*	\since version 0.0.2
		*/
		void setDisplayWeight(double _wt);

		/*! \brief This method sets number of sample points of the curve for display.
		*
		*	\param		[in]	_numPoints				- input num points.
		*	\since version 0.0.2
		*/
		void setDisplayNumPoints(int _numPoints = 100);

		//--------------------------
		//--- GET METHODS 
		//--------------------------

		/*! \brief This method computes the points on the curve.
		*
		*	\param		[in]	numPoints			- number of points required.
		*	\param		[out]	positions			- output container of positions
		*	\since version 0.0.4
		*/
		void getCurvePositions( int numPoints, zPointArray& positions);

		/*! \brief This method gets the control points of the curve.
		*
		*	\param		[out]	positions			- output container of control points positions
		*	\since version 0.0.4
		*/
		void getControlPoints(zPointArray& positions);

		/*! \brief This method gets pointer to the internal curve positions container.
		*
		*	\return				zPoint*					- pointer to internal vertex position container.
		*	\since version 0.0.4
		*/
		zPoint* getRawControlPoints();

		/*! \brief This method gets point at the specified parameter value.
		*
		*	\param		[in]	t						- parameter value ( between 0 & 1).
		*	\return				zPoint					- point at parameter value.
		*	\since version 0.0.4
		*/
		zPoint getPointAt(double t);

		/*! \brief This method gets tangent at the specified parameter value.
		*
		*	\param		[in]	t						- parameter value ( between 0 & 1).
		*	\return				zVector					- tangent at parameter value.
		*	\since version 0.0.4
		*/
		zVector getTangentAt(double t);

		/*! \brief This method gets tangent at the specified parameter value.
		*
		*	\param		[in]	t						- parameter value ( between 0 & 1).
		*	\param		[out]	p						- point at parameter value.
		*	\param		[out]	tan						- tangent at parameter value.
		*	\since version 0.0.4
		*/
		void getPointTangentAt(double t, zPoint &p, zVector &tan);

		/*! \brief This method gets pointer to the internal curve positions container.
		*
		*	\return				zPoint*					- pointer to internal vertex position container.
		*	\since version 0.0.4
		*/
		zPoint* getRawDisplayPositions();
		
		/*! \brief This method gets the center the curve.
		*
		*	\return		zPoint					- center .
		*	\since version 0.0.4
		*/
		zPoint getCenter();

		/*! \brief This method gets the length of the curve.
		*
		*	\return		double					- length of the curve .
		*	\since version 0.0.4
		*/
		double getLength();

		/*! \brief This method gets the domain of the curve.
		*
		*	\return		zDomainDouble					- domain of the curve .
		*	\since version 0.0.4
		*/
		zDomainDouble getDomain();

		/*! \brief This method gets the knot of the curve.
		*
		*	\return		zDoubleArray					- container of knot vector of the curve .
		*	\since version 0.0.4
		*/
		zDoubleArray getKnotVector();

		/*! \brief This method gets the degree of the curve.
		*
		*	\return		int					- degree of the curve .
		*	\since version 0.0.4
		*/
		int getDegree();
		
		/*! \brief This method computes the lengths of all the half edges of a the graph.
		*
		*	\param		[out]	halfEdgeLengths				- vector of halfedge lengths.
		*	\return				double						- total edge lengths.
		*	\since version 0.0.2
		*/
		ON_NurbsCurve* getRawON_Curve();

		
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
		void transformObject(zTransform &transform) override;


	private:

		//--------------------------
		//---- PRIVATE METHODS
		//--------------------------

		bool ON_NurbsCurve_GetLength(const ON_NurbsCurve& curve, double* length, double fractional_tolerance = 1.0e-8, const ON_Interval* sub_domain = NULL);
		
	};


}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterop/functionsets/zFnNurbsCurve.cpp>
#endif

#endif