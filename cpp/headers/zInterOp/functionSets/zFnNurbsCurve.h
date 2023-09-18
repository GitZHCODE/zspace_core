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

#include<headers/zInterface/functionsets/zFn.h>
#include<headers/zInterop/objects/zObjNurbsCurve.h>

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
		/*!	\brief boolean true is its a planar graph  */
		bool planarGraph;

		/*!	\brief stores normal of the the graph if planar  */
		zVector graphNormal;

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
		void create(zPointArray(&_positions), int degree,  bool periodic, int displayNumPoints = 20);
				

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

		void intersect(zPlane &plane, zPoint &intersectionPt, double &t);
		

		//--------------------------
		//--- SET METHODS 
		//--------------------------

		/*! \brief This method sets degree of the curve.
		*
		*	\param		[in]	_degree				- input degree.
		*	\since version 0.0.2
		*/
		void setDegree(int _degree);


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
		*	\param		[out]	numPoints				- output number of points in container.
		*	\return				zPoint*					- pointer to internal vertex position container.
		*	\since version 0.0.4
		*/
		zPoint* getRawDisplayPositions(int& numPoints);
		
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