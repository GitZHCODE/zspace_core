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

		/*!	\brief core utilities Object  */
		zUtilsCore coreUtils;

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
		void create(zPointArray(&_positions), int degree,  bool periodic, int displayNumPoints = 25);
				

		//--------------------------
		//--- TOPOLOGY QUERY METHODS 
		//--------------------------

		
	

		//--------------------------
		//--- COMPUTE METHODS 
		//--------------------------

		

		//--------------------------
		//--- SET METHODS 
		//--------------------------

		/*! \brief This method sets vertex positions of all the vertices.
		*
		*	\param		[in]	pos				- positions  contatiner.
		*	\since version 0.0.2
		*/
		void setDegree(int _degree);


		/*! \brief This method sets curve color to the input color.
		*
		*	\param		[in]	col				- input color.
		*	\since version 0.0.2
		*/
		void setCurveColor(zColor col);

	
		/*! \brief This method sets edge weight of the curve to the input weight.
		*
		*	\param		[in]	wt				- input weight.
		*	\since version 0.0.2
		*/
		void setCurveWeight(double wt);

		//--------------------------
		//--- GET METHODS 
		//--------------------------

		/*! \brief This method gets curve positions.
		*
		*	\param		[out]	pos				- positions  contatiner.
		*	\since version 0.0.2
		*/
		void getCurvePositions(zPointArray& pos);

		/*! \brief This method gets pointer to the internal curve positions container.
		*
		*	\return				zPoint*					- pointer to internal vertex position container.
		*	\since version 0.0.2
		*/
		zPoint* getRawCurvePositions();

		
		/*! \brief This method computes the center the graph.
		*
		*	\return		zPoint					- center .
		*	\since version 0.0.2
		*/
		zPoint getCenter();

		
		/*! \brief This method computes the lengths of all the half edges of a the graph.
		*
		*	\param		[out]	halfEdgeLengths				- vector of halfedge lengths.
		*	\return				double						- total edge lengths.
		*	\since version 0.0.2
		*/
		ON_NurbsCurve* getRawON_Curve();

		
		//--------------------------
		//---- TOPOLOGY MODIFIER METHODS
		//--------------------------
		
		
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

		//--------------------------
		//---- PROTECTED REMOVE INACTIVE
		//--------------------------

		//--------------------------
		//---- PROTECTED FACTORY METHODS
		//--------------------------


	private:

		//--------------------------
		//---- PRIVATE METHODS
		//--------------------------

	};


}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<source/zInterop/functionsets/zFnNurbsCurve.cpp>
#endif

#endif