// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Authors : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>, Leo Bieling <leo.bieling.zaha-hadid.com>
//

#ifndef ZSPACE_FN_MESHFIELD_H
#define ZSPACE_FN_MESHFIELD_H

#pragma once

#include <zspace/zInterface/objects/zObjectGraph.h>
#include <zspace/zInterface/objects/zObjectMesh.h>
#include <zspace/zInterface/objects/zObjectPointCloud.h>
#include <zspace/zInterface/objects/zObjectMeshField.h>

#include <zspace/zInterface/functionsets/zFnMesh.h>
#include <zspace/zInterface/functionsets/zFnGraph.h>
#include <zspace/zInterface/functionsets/zFnPointCloud.h>

#include <type_traits>

namespace zSpace
{
	class zFnMeshVectorField;

	/** \addtogroup zInterface
	*	\brief The Application Program Interface of the library.
	*  @{
	*/

	/** \addtogroup zFuntionSets
	*	\brief The function set classes of the library.
	*  @{
	*/

	/*! \class zFnMeshScalarField
	*	\brief A concrete 2D scalar mesh field function set.
	*
	*	\since version 0.0.5
	*/
	class ZSPACE_API zFnMeshScalarField
	{
		friend class zFnMeshVectorField;

	private:
		zFnType fnType;
		zDomain<float> contourValueDomain;
		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<float> contourVertexValues;

	protected:
		zUtilsCore coreUtils;
		bool setValuesperVertex = true;
		bool triMesh = true;
		zDomainColor fieldColorDomain = zDomainColor(zColor(), zColor(1, 1, 1, 1));

	public:
		zObjectMeshScalarField* fieldObj;
		zFnMesh fnMesh;

		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<zIntArray> ringNeighbours;

		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<zIntArray> adjacentNeighbours;

		//--------------------------
		//---- CONSTRUCTOR / DESTRUCTOR
		//--------------------------

		zFnMeshScalarField();
		zFnMeshScalarField(zObjectMeshScalarField& _fieldObj);
		~zFnMeshScalarField();

		//--------------------------
		//---- OVERRIDE / LIFECYCLE METHODS
		//--------------------------

		zFnType getType();
		void getBounds(zPoint& minBB, zPoint& maxBB);
		void clear();

		//--------------------------
		//---- CREATE METHODS
		//--------------------------

		void create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _NR = 1, bool _setValuesperVertex = true, bool _triMesh = true);
		void create(double _unit_X, double _unit_Y, int _n_X, int _n_Y, zPoint _minBB = zPoint(), int _NR = 1, bool _setValuesperVertex = true, bool _triMesh = true);

		//--------------------------
		//---- DIRECT GRID & INDEX QUERIES
		//--------------------------

		zObjectMesh* getRawMesh();
		int getIndex(int index_X, int index_Y) const;
		void getIndices(int index, int& index_X, int& index_Y) const;
		zPoint getPosition(int index) const;
		zPoint getPosition(int index_X, int index_Y) const;
		void getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const;
		void getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const;
		void getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour);
		void getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour);

		//--------------------------
		//---- GET METHODS
		//--------------------------

		int numFieldValues() const;
		void getResolution(int& _n_X, int& _n_Y) const;
		void getUnitDistances(double& _unit_X, double& _unit_Y) const;
		void getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const;
		void getPositions(zPointArray& positions);

		zScalar getFieldValue(int index) const;
		bool getFieldValue(zPoint& samplePos, zFieldValueType type, zScalar& fieldValue);
		bool getScalarValue(zScalarArray& scalars, zPoint& samplePos, zFieldValueType type, zScalar& fieldValue);

		void getFieldValues(zScalarArray& fieldValues);
		zScalar* getRawFieldValues();

		zVector getGradient(int index, float epsilon = EPS);
		zVectorArray getGradients(float epsilon = EPS);

		bool getValuesPerVertexBoolean() const;
		bool getTriMeshBoolean() const;
		void getFieldColorDomain(zDomainColor& colDomain);

		//--------------------------
		//---- SET METHODS
		//--------------------------

		void setFieldColorDomain(zDomainColor& colDomain);
		void setBoundingBox(zPoint& _minBB, zPoint& _maxBB);
		void setFieldValue(int index, zScalar val, bool append = false);
		void setFieldValues(zScalarArray& fValues, zFieldColorType type = zFieldColorType::zFieldRegular, float sdfWidth = 0.1f);

		//--------------------------
		//---- 2D IDW FIELD METHODS
		//--------------------------

		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectMesh& inMeshObj, zScalar meshValue, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectGraph& inGraphObj, zScalar graphValue, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalar value, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zObjectPointCloud& inPointsObj, zScalarArray& values, vector<double>& influences, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalar value, double influence, double power = 2.0, bool normalise = true);
		void getFieldValuesAsVertexDistance_IDW(zScalarArray& fieldValues, zPointArray& inPositions, zScalarArray& values, vector<double>& influences, double power = 2.0, bool normalise = true);

		//--------------------------
		//---- 2D SCALAR DISTANCE METHODS
		//--------------------------

		void getScalarsAsVertexDistance(zScalarArray& scalars, zPointArray& inPositions, bool normalise = true);
		void getScalarsAsVertexDistance(zScalarArray& scalars, zPointArray& inPositions, float offset, bool normalise = true);
		void getScalarsAsVertexDistance(zScalarArray& scalars, zObjectPointCloud& inPointsObj, bool normalise = true);
		void getScalarsAsVertexDistance(zScalarArray& scalars, zObjectPointCloud& inPointsObj, float offset, bool normalise = true);
		void getScalarsAsVertexDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, float offset, bool normalise = true);
		void getScalarsAsVertexDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, float offset, bool normalise = true);
		void getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectMesh& inMeshObj, float offset, bool normalise = true);
		void getScalarsAsEdgeDistance(zScalarArray& scalars, zObjectGraph& inGraphObj, float offset, bool normalise = true);

		//--------------------------
		//---- 2D SDF PRIMITIVE METHODS
		//--------------------------

		void getScalars_Polygon(zScalarArray& scalars, zObjectGraph& inGraphObj, bool normalise = true);
		void getScalars_Circle(zScalarArray& scalars, zVector& cen, float r, double annularVal = 0, bool normalise = true);
		void getScalars_Ellipse(zScalarArray& scalars, zVector& cen, float a, float b, double annularVal = 0, bool normalise = true);
		void getScalars_Line(zScalarArray& scalars, zVector& v0, zVector& v1, double annularVal = 0, bool normalise = true);
		void getScalars_Triangle(zScalarArray& scalars, zPoint& p0, zPoint& p1, zPoint& p2, double annularVal = 0, bool normalise = true);
		void getScalars_Square(zScalarArray& scalars, zVector& cen, zVector& dimensions, float annularVal = 0, bool normalise = true);
		void getScalars_Trapezoid(zScalarArray& scalars, float r1, float r2, float he, float annularVal = 0, bool normalise = true);

		//--------------------------
		//---- COMPUTE & SMOOTH METHODS
		//--------------------------

		bool checkPositionBounds(zPoint& pos, int& index);
		bool checkBounds_X(int index_X) const;
		bool checkBounds_Y(int index_Y) const;
		void computeDomain(zScalarArray& values, zDomain<zScalar>& domain);
		void normliseValues(zScalarArray& values);
		void smoothField(zScalarArray& scalars, int numSmooth, double diffuseDamp = 1.0, zDiffusionType type = zAverage);
		void computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions);
		void computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies);
		double F_of_r(double& r, double& a, double& b);

		//--------------------------
		//---- BLEND & BOOLEAN METHODS
		//--------------------------

		void blend_linear(int currentFrame, int totalFrames, zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result);
		void boolean_union(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise = true);
		void boolean_subtract(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise = true);
		void boolean_intersect(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise = true);
		void boolean_difference(zScalarArray& fieldValues_A, zScalarArray& fieldValues_B, zScalarArray& fieldValues_Result, bool normalise = true);
		void boolean_clipwithPlane(zScalarArray& scalars, zPlane& clipPlane);
		void boolean_clipwithPlane(zScalarArray& fieldValues_A, zScalarArray& fieldValues_Result, zPoint& clipPlane_center, zVector& clipPlane_Norm);

		//--------------------------
		//---- UPDATE & CONTOUR METHODS
		//--------------------------

		void updateColors(zFieldColorType type = zFieldColorType::zFieldRegular, float sdfWidth = 0.1f);
		void getIsocontour(zObjectGraph& coutourGraphObj, float inThreshold, int precision = PRECISION, float distTolerance = distanceTolerance);
		void getIsolineMesh(zObjectMesh& coutourMeshObj, float inThreshold = 0.5f, bool invertMesh = false);
		void getIsobandMesh(zObjectMesh& coutourMeshObj, float inThresholdLow = 0.2f, float inThresholdHigh = 0.5f);

	protected:
		void createFieldMesh();
		float getScalar_Polygon(zObjectGraph& inGraphObj, zPoint& p);
		float getScalar_Circle(zPoint& cen, zPoint& p, float r);
		float getScalar_Ellipse(zPoint& cen, zPoint& p, float a, float b);
		float getScalar_Line(zPoint& p, zPoint& v0, zPoint& v1);
		float getScalar_Triangle(zPoint& p, zPoint& p0, zPoint& p1, zPoint& p2);
		float getScalar_Square(zPoint& p, zVector& cen, zVector& dimensions);
		double getScalar_Trapezoid(zPoint& p, float& r1, float& r2, float& he);
	};

	/*! \class zFnMeshVectorField
	*	\brief A concrete 2D vector mesh field function set.
	*
	*	\since version 0.0.5
	*/
	class ZSPACE_API zFnMeshVectorField
	{
	private:
		zFnType fnType;

	protected:
		zUtilsCore coreUtils;
		bool setValuesperVertex = true;
		bool triMesh = true;
		zDomainColor fieldColorDomain = zDomainColor(zColor(), zColor(1, 1, 1, 1));

	public:
		zObjectMeshVectorField* fieldObj;
		zFnMesh fnMesh;

		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<zIntArray> ringNeighbours;

		ZSPACE_SUPPRESS_DLL_INTERFACE
		vector<zIntArray> adjacentNeighbours;

		//--------------------------
		//---- CONSTRUCTOR / DESTRUCTOR
		//--------------------------

		zFnMeshVectorField();
		zFnMeshVectorField(zObjectMeshVectorField& _fieldObj);
		~zFnMeshVectorField();

		//--------------------------
		//---- OVERRIDE / LIFECYCLE METHODS
		//--------------------------

		zFnType getType();
		void getBounds(zPoint& minBB, zPoint& maxBB);
		void clear();

		//--------------------------
		//---- CREATE METHODS
		//--------------------------

		void create(zPoint _minBB, zPoint _maxBB, int _n_X, int _n_Y, int _NR = 1, bool _setValuesperVertex = true, bool _triMesh = true);
		void create(double _unit_X, double _unit_Y, int _n_X, int _n_Y, zPoint _minBB = zPoint(), int _NR = 1, bool _setValuesperVertex = true, bool _triMesh = true);
		void createVectorFromScalarField(zObjectMeshScalarField& scalarFieldObj);

		//--------------------------
		//---- DIRECT GRID & INDEX QUERIES
		//--------------------------

		zObjectMesh* getRawMesh();
		int getIndex(int index_X, int index_Y) const;
		void getIndices(int index, int& index_X, int& index_Y) const;
		zPoint getPosition(int index) const;
		zPoint getPosition(int index_X, int index_Y) const;
		void getNeighbour_Ring(int index, int numRings, zIntArray& outRingNeighbours) const;
		void getNeighbour_Adjacents(int index, zIntArray& outAdjacentNeighbours) const;
		void getNeighbour_Contained(zPoint& pos, zIntArray& containedNeighbour);
		void getNeighbourPosition_Contained(zPoint& pos, zPointArray& containedNeighbour);

		//--------------------------
		//---- GET METHODS
		//--------------------------

		int numFieldValues() const;
		void getResolution(int& _n_X, int& _n_Y) const;
		void getUnitDistances(double& _unit_X, double& _unit_Y) const;
		void getBoundingBox(zPoint& _minBB, zPoint& _maxBB) const;
		void getPositions(zPointArray& positions);

		zVector getFieldValue(int index) const;
		bool getFieldValue(zPoint& samplePos, zFieldValueType type, zVector& fieldValue);

		void getFieldValues(zVectorArray& fieldValues);
		zVector* getRawFieldValues();

		bool getValuesPerVertexBoolean() const;
		bool getTriMeshBoolean() const;
		void getFieldColorDomain(zDomainColor& colDomain);

		//--------------------------
		//---- SET METHODS
		//--------------------------

		void setFieldColorDomain(zDomainColor& colDomain);
		void setBoundingBox(zPoint& _minBB, zPoint& _maxBB);
		void setFieldValue(int index, const zVector& val, bool append = false);
		void setFieldValues(zVectorArray& fValues, zFieldColorType type = zFieldColorType::zFieldRegular, float sdfWidth = 0.1f);

		//--------------------------
		//---- 2D IDW FIELD METHODS
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

		bool checkPositionBounds(zPoint& pos, int& index);
		bool checkBounds_X(int index_X) const;
		bool checkBounds_Y(int index_Y) const;
		void computeDomain(zVectorArray& values, zDomain<zVector>& domain);
		void normliseValues(zVectorArray& values);
		void computePositionsInFieldIndex(zPointArray& positions, vector<zPointArray>& fieldIndexPositions);
		void computePositionIndicesInFieldIndex(zPointArray& positions, vector<zIntArray>& fieldIndexPositionIndicies);
		double F_of_r(double& r, double& a, double& b);

	protected:
		void createFieldMesh();
	};

	template<typename T>
	using zFnMeshField = std::conditional_t<std::is_same_v<T, zScalar>, zFnMeshScalarField, zFnMeshVectorField>;

	template<typename T>
	using zFnMeshFieldBase = zFnMeshField<T>;

	/** @}*/

	/** @}*/

}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include<src/zInterface/functionsets/zFnMeshField.cpp>
#endif

#endif
