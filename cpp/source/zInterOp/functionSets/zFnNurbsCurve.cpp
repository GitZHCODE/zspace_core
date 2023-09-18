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


#include<headers/zInterop/functionsets/zFnNurbsCurve.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnNurbsCurve::zFnNurbsCurve()
	{
		fnType = zFnType::zNurbsCurveFn;
		nurbsCurveObj = nullptr;
	}

	ZSPACE_INLINE zFnNurbsCurve::zFnNurbsCurve(zObjNurbsCurve&_nurbsCurveObj)
	{
		fnType = zFnType::zNurbsCurveFn;

		nurbsCurveObj = &_nurbsCurveObj;

		
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnNurbsCurve::~zFnNurbsCurve() {}

	//---- OVERRIDE METHODS
	
	ZSPACE_INLINE zFnType zFnNurbsCurve::getType()
	{
		return zNurbsCurveFn;
	}

	ZSPACE_INLINE void zFnNurbsCurve::from(string path, zFileTpye type, bool staticGeom)
	{
		if (type == zJSON)
		{
			json j;

			bool chk = coreUtils.readJSON(path, j);
			if (chk) from(j, staticGeom);			
		}

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnNurbsCurve::from(json& j, bool staticGeom)
	{
		zUtilsJsonHE graphJSON;
		
		// read data to json 
		

		// Curve Attributes
		vector<zDoubleArray> cPoints;		
		readJSONAttribute(j, "NC_controlPoints", cPoints);

		zDoubleArray cPointWeights;
		readJSONAttribute(j, "NC_weights", cPointWeights);
		

		int cDegree;
		readJSONAttribute(j, "NC_degree", cDegree);
		nurbsCurveObj->degree = cDegree;

		bool cPeriodic;
		readJSONAttribute(j, "NC_periodic", cPeriodic);
		nurbsCurveObj->periodic = cPeriodic;
		
		//printf("\n vertexAttributes: %zi %zi", vertexAttributes.size(), vertexAttributes[0].size());
		nurbsCurveObj->controlPoints.clear();
		nurbsCurveObj->displayPositions.clear();
		nurbsCurveObj->controlPointWeights.clear();

		ON_3dPointArray points;
		
		for (int i = 0; i < cPoints.size(); i++)
		{
			nurbsCurveObj->controlPointWeights.push_back(cPointWeights[i]);
			points.Append(ON_3dPoint(cPoints[i][0], cPoints[i][1], cPoints[i][2]));			
		}

		const int order = cDegree + 1;
			
		bool check =	(cPeriodic) ? nurbsCurveObj->curve.CreatePeriodicUniformNurbs(3, order, points.Count(), points) : nurbsCurveObj->curve.CreateClampedUniformNurbs(3, order, points.Count(), points);
		printf("\n curve created %s ", (check) ? "True" : "False");

		// store control points
		for (int i = 0; i < points.Count(); i++)
		{
			//nurbsCurveObj->curve.SetCV(i, points[i]);
			nurbsCurveObj->curve.SetWeight(i, cPointWeights[i]);			
			
			zPoint cP(points[i].x, points[i].y, points[i].z);
			nurbsCurveObj->controlPoints.push_back(cP);			
		}
		
		// normalise curve
		nurbsCurveObj->curve.SetDomain(0.0, 1.0);
		
		// display points 
		setDisplayNumPoints();
			
		
	}

	ZSPACE_INLINE void zFnNurbsCurve::to(string path, zFileTpye type)
	{
		if (type == zJSON)
		{
			json j;
			to(j);
			bool chk = coreUtils.writeJSON(path, j);
			//toJSON(path);
		}

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnNurbsCurve::to(json& j)
	{
		vector<zDoubleArray> cPoints;
		cPoints.assign(nurbsCurveObj->controlPoints.size(), zDoubleArray());

		zDoubleArray cPointWeights;
		cPointWeights.assign(nurbsCurveObj->controlPoints.size(), -1);

		for (int i = 0; i < nurbsCurveObj->controlPoints.size(); i++)
		{
			cPoints[i].push_back(nurbsCurveObj->controlPoints[i].x);
			cPoints[i].push_back(nurbsCurveObj->controlPoints[i].y);
			cPoints[i].push_back(nurbsCurveObj->controlPoints[i].z);

			cPointWeights[i] = nurbsCurveObj->controlPointWeights[i];
		}

		int cDegree = nurbsCurveObj->degree;
		bool cPeriodic = nurbsCurveObj->periodic;		

		zDomainDouble domain = getDomain();
		zDoubleArray cDomain = { domain.min, domain.max};

		zDoubleArray cKnots = getKnotVector();


		// Json file 
		coreUtils.writeJSONAttribute(j, "NC_controlPoints", cPoints);
		coreUtils.writeJSONAttribute(j, "NC_weights", cPointWeights);
		coreUtils.writeJSONAttribute(j, "NC_degree", cDegree);
		coreUtils.writeJSONAttribute(j, "NC_domain", cDomain);
		coreUtils.writeJSONAttribute(j, "NC_knots", cKnots);
		coreUtils.writeJSONAttribute(j, "NC_periodic", cPeriodic);
		
	}

	ZSPACE_INLINE void zFnNurbsCurve::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		nurbsCurveObj->getBounds(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnNurbsCurve::clear()
	{
		nurbsCurveObj->controlPoints.clear();
		nurbsCurveObj->displayPositions.clear();
	}

	//---- CREATE METHODS

	ZSPACE_INLINE void zFnNurbsCurve::create(zPointArray(&_positions), int degree, bool periodic, int displayNumPoints)
	{
		nurbsCurveObj->controlPoints.clear();
		//nurbsCurveObj->controlPoints = _positions;

		nurbsCurveObj->displayPositions.clear();
		nurbsCurveObj->controlPoints.clear();

		ON_3dPointArray points;

		for (int i = 0; i < _positions.size(); i++)
		{
			points.Append(ON_3dPoint(_positions[i][0], _positions[i][1], _positions[i][2]));
		}

		(periodic) ? nurbsCurveObj->curve.CreatePeriodicUniformNurbs(degree, degree - 1, points.Count(), points) : nurbsCurveObj->curve.CreateClampedUniformNurbs(degree, degree - 1, points.Count(), points);


		for (int i = 0; i < nurbsCurveObj->curve.CVCount(); i++)
		{
			nurbsCurveObj->controlPointWeights.push_back(nurbsCurveObj->curve.Weight(i));
		
			ON_3dPoint p;
			nurbsCurveObj->curve.GetCV(i, p);

			zPoint cP(p.x, p.y, p.z);
			nurbsCurveObj->controlPoints.push_back(cP);
		}

		// normalise curve
		nurbsCurveObj->curve.SetDomain(0.0, 1.0);

		// display points 
		setDisplayNumPoints();
	}

	ZSPACE_INLINE int zFnNurbsCurve::numControlVertices()
	{
		return nurbsCurveObj->controlPoints.size();
	}

	ZSPACE_INLINE void zFnNurbsCurve::intersect(zPlane& plane, zPoint& intersectionPt, double& t)
	{
		//ON_3dPoint o = coreUtils.
	}

	//--- SET METHODS 

	ZSPACE_INLINE void zFnNurbsCurve::setDegree(int _degree)
	{
		nurbsCurveObj->curve.IncreaseDegree(_degree);
		nurbsCurveObj->degree = _degree;

		// display points 
		int numPoints = nurbsCurveObj->displayPositions.size();
		getCurvePositions(numPoints, nurbsCurveObj->displayPositions);
	}

	ZSPACE_INLINE void zFnNurbsCurve::setDisplayColor(zColor _col)
	{
		nurbsCurveObj->displayColor = _col;
	}

	ZSPACE_INLINE void zFnNurbsCurve::setDisplayWeight(double _wt)
	{
		nurbsCurveObj->displayWeight = _wt;
	}

	ZSPACE_INLINE void zFnNurbsCurve::setDisplayNumPoints(int _numPoints)
	{
		getCurvePositions(_numPoints, nurbsCurveObj->displayPositions);
	}


	//--- GET METHODS 

	ZSPACE_INLINE void zFnNurbsCurve::getCurvePositions(int numPoints, zPointArray& positions)
	{
		positions.clear();

		if (nurbsCurveObj->degree == 1)
		{
			positions = nurbsCurveObj->controlPoints;
		}
		else
		{
			zDoubleArray s;
			s.assign(numPoints, -1);

			s[0] = 0;
			for (int i = 1; i < numPoints - 1; i++) s[i] = (1.0 / (numPoints - 1)) * i;
			s[numPoints - 1] = 1;

			for (int i = 0; i < numPoints; i++)
			{
				ON_3dPoint p = nurbsCurveObj->curve.PointAt(s[i]);
				positions.push_back(zPoint(p.x, p.y, p.z));
			}
		}

		
	}

	ZSPACE_INLINE zPoint* zFnNurbsCurve::getRawControlPoints()
	{
		return &nurbsCurveObj->controlPoints[0];
	}

	ZSPACE_INLINE zPoint zFnNurbsCurve::getPointAt(double t)
	{
		ON_3dPoint p = nurbsCurveObj->curve.PointAt(t);
		return zPoint(p.x, p.y, p.z);
	}

	ZSPACE_INLINE zVector zFnNurbsCurve::getTangentAt(double t)
	{
		ON_3dVector tan = nurbsCurveObj->curve.TangentAt(t);
		return zVector(tan.x, tan.y, tan.z);
	}

	ZSPACE_INLINE void zFnNurbsCurve::getPointTangentAt(double t, zPoint& p, zVector& tan)
	{
		p = getPointAt(t);
		tan = getTangentAt(t);
	}

	ZSPACE_INLINE zPoint* zFnNurbsCurve::getRawDisplayPositions(int& numPoints)
	{
		numPoints = nurbsCurveObj->displayPositions.size();
		return &nurbsCurveObj->displayPositions[0];
	}

	ZSPACE_INLINE zPoint zFnNurbsCurve::getCenter()
	{
		ON_3dPoint p = nurbsCurveObj->curve.PointAt(0.5);
		return zPoint(p.x, p.y, p.z);
	}

	ZSPACE_INLINE double zFnNurbsCurve::getLength()
	{
		double len = 0.0;
		ON_NurbsCurve nCurve = nurbsCurveObj->curve;
		ON_NurbsCurve_GetLength(nCurve, &len);

		return len;
	
	}

	ZSPACE_INLINE zDomainDouble zFnNurbsCurve::getDomain()
	{
		zDomainDouble out;
		nurbsCurveObj->curve.GetDomain(&out.min, &out.max);

		return out;
	}

	ZSPACE_INLINE zDoubleArray zFnNurbsCurve::getKnotVector()
	{
		zDoubleArray out;
		out.assign(nurbsCurveObj->curve.KnotCount(), -1);
		
		for (int i = 0; i < nurbsCurveObj->curve.KnotCount(); i++)
		{
			out[i] = nurbsCurveObj->curve.m_knot[i];
		}
		

		return out;
	}

	ZSPACE_INLINE int zFnNurbsCurve::getDegree()
	{
		return nurbsCurveObj->degree;
	}

	ZSPACE_INLINE ON_NurbsCurve* zFnNurbsCurve::getRawON_Curve()
	{
		return &nurbsCurveObj->curve;
	}


	//---- TRANSFORM METHODS OVERRIDES

	ZSPACE_INLINE void zFnNurbsCurve::setTransform(zTransform &inTransform, bool decompose, bool updatePositions)
	{
		if (updatePositions)
		{
			zTransformationMatrix to;
			to.setTransform(inTransform, decompose);

			zTransform transMat = nurbsCurveObj->transformationMatrix.getToMatrix(to);
			transformObject(transMat);

			nurbsCurveObj->transformationMatrix.setTransform(inTransform);

			// update pivot values of object transformation matrix
			zVector p = nurbsCurveObj->transformationMatrix.getPivot();
			p = p * transMat;
			setPivot(p);

		}
		else
		{
			nurbsCurveObj->transformationMatrix.setTransform(inTransform, decompose);

			zVector p = nurbsCurveObj->transformationMatrix.getO();
			setPivot(p);

		}

	}

	ZSPACE_INLINE void zFnNurbsCurve::setScale(zFloat4 &scale)
	{
		// get  inverse pivot translations
		zTransform invScalemat = nurbsCurveObj->transformationMatrix.asInverseScaleTransformMatrix();

		// set scale values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setScale(scale);

		// get new scale transformation matrix
		zTransform scaleMat = nurbsCurveObj->transformationMatrix.asScaleTransformMatrix();

		// compute total transformation
		zTransform transMat = invScalemat * scaleMat;

		// transform object
		transformObject(transMat);
	}

	ZSPACE_INLINE void zFnNurbsCurve::setRotation(zFloat4 &rotation, bool appendRotations)
	{
		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = nurbsCurveObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = nurbsCurveObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = nurbsCurveObj->transformationMatrix;
		to.setRotation(rotation, appendRotations);
		zTransform toMat = nurbsCurveObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set rotation values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setRotation(rotation, appendRotations);;
	}

	ZSPACE_INLINE void zFnNurbsCurve::setTranslation(zVector &translation, bool appendTranslations)
	{
		// get vector as zDouble3
		zFloat4 t;
		translation.getComponents(t);

		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = nurbsCurveObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = nurbsCurveObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = nurbsCurveObj->transformationMatrix;
		to.setTranslation(t, appendTranslations);
		zTransform toMat = nurbsCurveObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set translation values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setTranslation(t, appendTranslations);;

		// update pivot values of object transformation matrix
		zVector p = nurbsCurveObj->transformationMatrix.getPivot();
		p = p * transMat;
		setPivot(p);

	}

	ZSPACE_INLINE void zFnNurbsCurve::setPivot(zVector &pivot)
	{
		// get vector as zDouble3
		zFloat4 p;
		pivot.getComponents(p);

		// set pivot values of object transformation matrix
		nurbsCurveObj->transformationMatrix.setPivot(p);
	}

	ZSPACE_INLINE void zFnNurbsCurve::getTransform(zTransform &transform)
	{
		transform = nurbsCurveObj->transformationMatrix.asMatrix();
	}

	//---- PROTECTED OVERRIDE METHODS

	ZSPACE_INLINE void zFnNurbsCurve::transformObject(zTransform &transform)
	{

		if (numControlVertices() == 0) return;

	
		zPoint* pos = getRawControlPoints();

		for (int i = 0; i < numControlVertices(); i++)
		{

			zVector newPos = pos[i] * transform;
			pos[i] = newPos;
		}

		for (int i = 0; i < nurbsCurveObj->curve.CVCount(); i++)
		{
			ON_3dPoint p(pos[i].x, pos[i].y, pos[i].z);
			nurbsCurveObj->curve.SetCV(i, p);
		}

		// display points 
		int numPoints = nurbsCurveObj->displayPositions.size();
		getCurvePositions(numPoints, nurbsCurveObj->displayPositions);

	}


	//---- PRIVATE METHODS

	ZSPACE_INLINE bool zFnNurbsCurve::ON_NurbsCurve_GetLength(const ON_NurbsCurve& curve, double* length, double fractional_tolerance, const ON_Interval* sub_domain)
	{
		return curve.GetLength(length, fractional_tolerance, sub_domain);
	}


}