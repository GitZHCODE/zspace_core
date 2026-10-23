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

#ifndef ZSPACE_FN_POINTFIELD_H
#define ZSPACE_FN_POINTFIELD_H

#pragma once

#include <zspace/zInterface/objects/zObjectPointCloud.h>
#include <zspace/zInterface/objects/zObjectGraph.h>
#include <zspace/zInterface/objects/zObjectMesh.h>
#include <zspace/zInterface/objects/zObjectPointField.h>

#include <zspace/zInterface/functionsets/zFnMesh.h>
#include <zspace/zInterface/functionsets/zFnGraph.h>
#include <zspace/zInterface/functionsets/zFnPointCloud.h>

#include <type_traits>

namespace zSpace
{
	class zFnPointVectorField;

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zFuntionSets
	*	\brief The function set classes of the library.
	*  @{
	*/

	/*! \class zFnPointScalarField
	*	\brief A concrete 3D scalar point field function set.
	*
	*	\since version 0.0.5
	*/
	class ZSPACE_API zFnPointScalarField
	{
		friend class zFnPointVectorField;

	protected:
		zFnType fnType;
		zUtilsCore coreUtils;
		zDomain<float> contourValueDomain;
		zObjectPointScalarField* fieldObj;

	public:
		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<vector<int>> ringNeighbours;

		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<vector<int>> adjacentNeighbours;

		zFnPointCloud fnPoints;
		zDomainColor fieldColorDomain = zDomainColor(zColor(), zColor(1, 1, 1, 1));

		//--------------------------
		//---- CONSTRUCTOR / DESTRUCTOR
		//--------------------------

		zFnPointScalarField();
		zFnPointScalarField(zObjectPointScalarField& _fieldObj);
		~zFnPointScalarField();

		//--------------------------
		//---- OVERRIDE / LIFECYCLE METHODS
		//--------------------------

		zFnType getType();
		void getBounds(zPoint& minBB, zPoint& maxBB);
		void clear();

		//--------------------------
		//---- CREATE METHODS
		//--------------------------

		void create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _n_Z, int _NR = 1);
		void create(double _unit_X, double _unit_Y, double _unit_Z, int _n_X, int _n_Y, int _n_Z, zPoint _minBB = zPoint(), int _NR = 1);

		//--------------------------
		//---- DIRECT GRID & INDEX QUERIES
		//--------------------------

		int getIndex(int index_X, int index_Y, int index_Z) const;
		void getIndices(int index, int& index_X, int& index_Y, int& index_Z) const;
		zPoint getPosition(int index) const;
		zPoint getPosition(int index_X, int index_Y, int index_Z) const;
		void getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const;
		void getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const;
		void getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour);
		void getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour);

		//--------------------------
		//---- GET METHODS
		//--------------------------

		int numFieldValues() const;
		void getResolution(int& _n_X, int& _n_Y, int& _n_Z) const;
		void getUnitDistances(double& _unit_X, double& _unit_Y, double& _unit_Z) const;
		void getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const;
		void getPositions(zPointArray& positions);

		zScalar getFieldValue(int index) const;
		bool getFieldValue(zPoint& samplePos, zFieldValueType type, zScalar& fieldValue);
		void getFieldValues(zScalarArray& fieldValues);
		zScalar* getRawFieldValues();

		zVector getGradient(int index, float epsilon = EPS);
		vector<zVector> getGradients(float epsilon = EPS);

		//--------------------------
		//---- SET METHODS
		//--------------------------

		void setFieldColorDomain(zDomainColor& colDomain);
		void setBoundingBox(zPoint& _minBB, zPoint& _maxBB);
		void setFieldValue(int index, zScalar val, bool append = false);
		void setFieldValues(zScalarArray& fValues);

		//--------------------------
		//---- 3D IDW FIELD METHODS
		//--------------------------

		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectMesh& inMeshObj, zScalar meshValue, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectGraph& inGraphObj, zScalar graphValue, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalar value, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalarArray& values, vector<double>& influences, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalar value, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalarArray& values, vector<double>& influences, double power = 2.0, bool normalise = true);

		//--------------------------
		//---- 3D SCALAR DISTANCE METHODS
		//--------------------------

		void getScalarsAsVertexDistance(zScalarArray& scalars, zObjectPointCloud& inPointsObj, bool normalise = true);
		void getScalarsAsVertexDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, double offset, bool normalise = true);
		void getScalarsAsVertexDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, double offset, bool normalise = true);
		void getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, double offset, bool normalise = true);
		void getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, double offset, bool normalise = true);

		//--------------------------
		//---- COMPUTE & SMOOTH METHODS
		//--------------------------

		bool checkPositionBounds(zPoint& pos, int& index) const;
		bool checkBounds_X(int index_X) const;
		bool checkBounds_Y(int index_Y) const;
		bool checkBounds_Z(int index_Z) const;
		void computeMinMaxOfScalars(zScalarArray& values, zScalar& dMin, zScalar& dMax);
		void computeDomain(zScalarArray& values, zDomain<zScalar>& domain);
		void normliseValues(zScalarArray& values);
		void smoothField(int numSmooth, double diffuseDamp = 1.0, zDiffusionType type = zAverage);
		void computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions);
		void computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies);
		double F_of_r(double& r, double& a, double& b);

		//--------------------------
		//---- BOOLEAN & UPDATE METHODS
		//--------------------------

		void boolean_union(zScalarArray& scalars0, zScalarArray& scalars1, zScalarArray& scalarsResult, bool normalise = true);
		void boolean_subtract(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise = true);
		void boolean_intersect(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise = true);
		void boolean_difference(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise = false);
		void boolean_clipwithPlane(zScalarArray& scalars, zMatrix4& clipPlane);
		void updateColors();

	protected:
		void createPointCloud();
	};

	/*! \class zFnPointVectorField
	*	\brief A concrete 3D vector point field function set.
	*
	*	\since version 0.0.5
	*/
	class ZSPACE_API zFnPointVectorField
	{
	protected:
		zFnType fnType;
		zUtilsCore coreUtils;
		zObjectPointVectorField* fieldObj;

	public:
		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<vector<int>> ringNeighbours;

		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<vector<int>> adjacentNeighbours;

		zFnPointCloud fnPoints;
		zDomainColor fieldColorDomain = zDomainColor(zColor(), zColor(1, 1, 1, 1));

		//--------------------------
		//---- CONSTRUCTOR / DESTRUCTOR
		//--------------------------

		zFnPointVectorField();
		zFnPointVectorField(zObjectPointVectorField& _fieldObj);
		~zFnPointVectorField();

		//--------------------------
		//---- OVERRIDE / LIFECYCLE METHODS
		//--------------------------

		zFnType getType();
		void getBounds(zPoint& minBB, zPoint& maxBB);
		void clear();

		//--------------------------
		//---- CREATE METHODS
		//--------------------------

		void create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _n_Z, int _NR = 1);
		void create(double _unit_X, double _unit_Y, double _unit_Z, int _n_X, int _n_Y, int _n_Z, zPoint _minBB = zPoint(), int _NR = 1);
		void createVectorFromScalarField(zFnPointScalarField& inFnScalarField);
		void createVectorFromScalarField(zObjectPointScalarField& scalarFieldObj);

		//--------------------------
		//---- DIRECT GRID & INDEX QUERIES
		//--------------------------

		int getIndex(int index_X, int index_Y, int index_Z) const;
		void getIndices(int index, int& index_X, int& index_Y, int& index_Z) const;
		zPoint getPosition(int index) const;
		zPoint getPosition(int index_X, int index_Y, int index_Z) const;
		void getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const;
		void getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const;
		void getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour);
		void getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour);

		//--------------------------
		//---- GET METHODS
		//--------------------------

		int numFieldValues() const;
		void getResolution(int& _n_X, int& _n_Y, int& _n_Z) const;
		void getUnitDistances(double& _unit_X, double& _unit_Y, double& _unit_Z) const;
		void getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const;
		void getPositions(zPointArray& positions);

		zVector getFieldValue(int index) const;
		bool getFieldValue(zPoint& samplePos, zFieldValueType type, zVector& fieldValue);
		void getFieldValues(zVectorArray& fieldValues);
		zVector* getRawFieldValues();

		//--------------------------
		//---- SET METHODS
		//--------------------------

		void setFieldColorDomain(zDomainColor& colDomain);
		void setBoundingBox(zPoint& _minBB, zPoint& _maxBB);
		void setFieldValue(int index, const zVector& val, bool append = false);
		void setFieldValues(zVectorArray& fValues);

		//--------------------------
		//---- 3D IDW FIELD METHODS
		//--------------------------

		void getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectMesh& inMeshObj, zVector meshValue, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectGraph& inGraphObj, zVector graphValue, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectPointCloud& inPointsObj, zVector value, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zObjectPointCloud& inPointsObj, zVectorArray& values, vector<double>& influences, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zPointArray& inPositions, zVector value, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zVectorArray& fieldValues, zPointArray& inPositions, zVectorArray& values, vector<double>& influences, double power = 2.0, bool normalise = true);

		//--------------------------
		//---- COMPUTE METHODS
		//--------------------------

		bool checkPositionBounds(zPoint& pos, int& index) const;
		bool checkBounds_X(int index_X) const;
		bool checkBounds_Y(int index_Y) const;
		bool checkBounds_Z(int index_Z) const;
		void computeMinMaxOfScalars(zVectorArray& values, zVector& dMin, zVector& dMax);
		void computeDomain(zVectorArray& values, zDomain<zVector>& domain);
		void normliseValues(zVectorArray& values);
		void computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions);
		void computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies);
		double F_of_r(double& r, double& a, double& b);

	protected:
		void createPointCloud();
	};

	template<typename T>
	using zFnPointField = std::conditional_t<std::is_same_v<T, zScalar>, zFnPointScalarField, zFnPointVectorField>;

	template<typename T>
	using zFnPointFieldBase = zFnPointField<T>;

	/** @}*/

	/** @}*/

}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/functionsets/zFnPointField.cpp>
#endif

#endif
