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


#include<headers/zInterop/functionsets/zFnArc.h>
#include<headers/zInterOp/objects/zObjPlane.h>
#include<headers/zInterop/functionsets/zFnPlane.h>
#include<headers/zInterOp/objects/zObjNurbsCurve.h>
#include<headers/zInterop/functionsets/zFnNurbsCurve.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnArc::zFnArc()
	{
		fnType = zFnType::zArcFn;
		arcObj = nullptr;
	}

	ZSPACE_INLINE zFnArc::zFnArc(zObjArc& _arcObj)
	{
		fnType = zFnType::zArcFn;
		arcObj = &_arcObj;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnArc::~zFnArc() {}

	//---- OVERRIDE METHODS
	
	ZSPACE_INLINE zFnType zFnArc::getType()
	{
		return zArcFn;
	}

	ZSPACE_INLINE void zFnArc::from(string path, zFileTpye type, bool staticGeom)
	{
		if (type == zJSON)
		{
			json j;

			bool chk = json_read(path, j);
			if (chk) from(j, staticGeom);			
		}

		else throw std::invalid_argument("error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnArc::from(json& j, bool staticGeom)
	{
		/*zObjPlane oooPlane;
		create(oooPlane, 5.0, Z_PI);*/

		zUtilsJsonHE graphJSON;	

		zDoubleArray planeArray;
		double _radius;
		double _angle;

		json_readAttribute(j, "Plane", planeArray);
		json_readAttribute(j, "Radius", _radius);
		json_readAttribute(j, "Angle", _angle);

		zPoint origin(planeArray[3], planeArray[7], planeArray[11]);
		zVector xAxis(planeArray[0], planeArray[1], planeArray[2]);
		zVector yAxis(planeArray[4], planeArray[5], planeArray[6]);
	
		zFnPlane planeFn(arcObj->oPlane);
		planeFn.create(origin, xAxis, yAxis);
		arcObj->radius = _radius;
		arcObj->angle = _angle;

		ON_3dPoint o_pos = ON_3dPoint(arcObj->oPlane.origin.x, arcObj->oPlane.origin.y, arcObj->oPlane.origin.z);
		ON_3dVector x_dir = ON_3dVector(arcObj->oPlane.xAxis.x, arcObj->oPlane.xAxis.y, arcObj->oPlane.xAxis.z);
		ON_3dVector y_dir = ON_3dVector(arcObj->oPlane.yAxis.x, arcObj->oPlane.yAxis.y, arcObj->oPlane.yAxis.z);

		ON_Plane onPlane(o_pos, x_dir, y_dir);
		arcObj->on_arc.Create(onPlane, _radius, _angle);

		cout << "_radius" << _radius << endl;
		cout << "_angle" << _angle << endl;
		cout << "_angle" << arcObj->angle << endl;

		arcObj->controlPoints.clear();
		arcObj->arcPositions.clear();

		computeArcPositions();
		computeControlPoints();
	}
	
	ZSPACE_INLINE void zFnArc::to(string path, zFileTpye type)
	{
		if (type == zJSON)
		{
			json j;
			to(j);
			bool chk = json_write(path, j);
			//toJSON(path);
		}

		else throw std::invalid_argument(" error: invalid zFileTpye type");
	}

	ZSPACE_INLINE void zFnArc::to(json& j)
	{
		zDoubleArray planeArray;
		double _radius;
		double _angle;

		planeArray = arcObj->oPlane.getDoubleArray();
		_radius = arcObj->radius;
		_angle = arcObj->angle;

		json_writeAttribute(j, "Plane", planeArray);
		json_writeAttribute(j, "Radius", _radius);
		json_writeAttribute(j, "Angle", _angle);

		/*
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

		coreUtils.writeJSONAttribute(j, "NC_controlPoints", cPoints);
		coreUtils.writeJSONAttribute(j, "NC_weights", cPointWeights);
		coreUtils.writeJSONAttribute(j, "NC_degree", cDegree);
		coreUtils.writeJSONAttribute(j, "NC_domain", cDomain);
		coreUtils.writeJSONAttribute(j, "NC_knots", cKnots);
		coreUtils.writeJSONAttribute(j, "NC_periodic", cPeriodic);
		*/
	}

#if defined ZSPACE_USD_INTEROP

	ZSPACE_INLINE void zFnArc::from(UsdPrim& usd, bool staticGeom)
	{
	}

	ZSPACE_INLINE void zFnArc::to(UsdPrim& usd)
	{
	}

#endif

	ZSPACE_INLINE void zFnArc::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		arcObj->getBounds(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnArc::clear()
	{
		arcObj->controlPoints.clear();
		arcObj->arcPositions.clear();
	}

	//---- CREATE METHODS
	ZSPACE_INLINE bool zFnArc::create(zObjPlane& _plane, double _radius, double _angle)
	{
		bool success = false;

		if (_angle <= 0 || _angle > Z_TWO_PI) {			
			throw std::invalid_argument("error: invalid angle domain to create arc");
			return success;
		}
		else {
			arcObj->controlPoints.clear();
			arcObj->arcPositions.clear();

			arcObj->oPlane = _plane;
			arcObj->radius = _radius;
			arcObj->angle = _angle;
			if (arcObj->angle == Z_TWO_PI)
			{
				arcObj->isCircle = true;
			}
			else
			{
				arcObj->isCircle = false;
			}

			ON_3dPoint o_pos = ON_3dPoint(_plane.origin.x, _plane.origin.y, _plane.origin.z);
			ON_3dVector x_dir = ON_3dVector(_plane.xAxis.x, _plane.xAxis.y, _plane.xAxis.z);
			ON_3dVector y_dir = ON_3dVector(_plane.yAxis.x, _plane.yAxis.y, _plane.yAxis.z);

			ON_Plane onPlane(o_pos, x_dir, y_dir);
			arcObj->on_arc.Create(onPlane, _radius, _angle);

			computeArcPositions();
			computeControlPoints();

			return success = true;
		}		
	}

	ZSPACE_INLINE bool zFnArc::createFromPoints(zPoint p0, zPoint p1, zPoint p2)
	{
		bool success = false;

		ON_3dPoint on_p0 = ON_3dPoint(p0.x, p0.y, p0.z);
		ON_3dPoint on_p1 = ON_3dPoint(p1.x, p1.y, p1.z);
		ON_3dPoint on_p2 = ON_3dPoint(p2.x, p2.y, p2.z);
		ON_Arc test_arc;
		success = test_arc.Create(on_p0, on_p1, on_p2);

		if (success)
		{
			arcObj->controlPoints.clear();
			arcObj->arcPositions.clear();

			arcObj->on_arc = test_arc;
			arcObj->angle = test_arc.AngleRadians();
			arcObj->radius = test_arc.Radius();
			arcObj->oPlane.origin = zPoint(test_arc.Center().x, test_arc.Center().y, test_arc.Center().z);
			arcObj->oPlane.xAxis = zVector(test_arc.Plane().xaxis.x, test_arc.Plane().xaxis.y, test_arc.Plane().xaxis.z);
			arcObj->oPlane.yAxis = zVector(test_arc.Plane().yaxis.x, test_arc.Plane().yaxis.y, test_arc.Plane().yaxis.z);
			arcObj->oPlane.normal = zVector(test_arc.Plane().zaxis.x, test_arc.Plane().zaxis.y, test_arc.Plane().zaxis.z);
			arcObj->oPlane.on_plane = test_arc.Plane();
			if (arcObj->angle == Z_TWO_PI)
			{
				arcObj->isCircle = true;
			}
			else
			{
				arcObj->isCircle = false;
			}

			computeArcPositions();
			computeControlPoints();
		}
		else throw std::invalid_argument("error: invalid points to create arc");
		return success;
	}

	ZSPACE_INLINE bool zFnArc::createFromTangent(zPoint p0, zVector t1, zPoint p1)
	{
		return true;
	}
		
	ZSPACE_INLINE int zFnArc::numControlVertices()
	{
		return arcObj->controlPoints.size();
	}

	ZSPACE_INLINE void zFnArc::intersect(zPlane& plane, zPoint& intersectionPt, double& t)
	{
		//ON_3dPoint o = coreUtils.
	}

	//--- SET METHODS 

	ZSPACE_INLINE void zFnArc::setArcDisplayColor(zColor _col)
	{
		arcObj->arcDisplayColor = _col;
	}

	ZSPACE_INLINE void zFnArc::setArcDisplayWeight(double _wt)
	{
		arcObj->arcDisplayWeight = _wt;
	}

	ZSPACE_INLINE void zFnArc::setAngle(double _angle)
	{
		arcObj->angle = _angle;
		arcObj->on_arc = ON_Arc(arcObj->oPlane.on_plane, arcObj->radius, arcObj->angle);
		computeArcPositions();
		computeControlPoints();
	}

	ZSPACE_INLINE void zFnArc::setRadius(double _radius)
	{
		arcObj->radius = _radius;
		arcObj->on_arc = ON_Arc(arcObj->oPlane.on_plane, arcObj->radius, arcObj->angle);
		computeArcPositions();
		computeControlPoints();
	}

	//--- COMPUTE METHODS 

	ZSPACE_INLINE void zFnArc::computeArcPositions(int numPoints)
	{
		arcObj->arcPositions.clear();

		for (int i = 0; i < numPoints; i++)
		{
			double angle = (arcObj->angle / numPoints) * i;
			zPoint p = arcObj->oPlane.origin + arcObj->oPlane.xAxis * arcObj->radius * cos(angle) + arcObj->oPlane.yAxis * arcObj->radius * sin(angle);
			arcObj->arcPositions.push_back(p);
		}		
	}

	ZSPACE_INLINE void zFnArc::computeControlPoints()
	{
		arcObj->controlPoints.clear();

		int numSpans = 1;
		if (arcObj->angle > Z_HALF_PI * 3)
		{
			numSpans = 4;
		}
		else if (arcObj->angle > Z_HALF_PI * 2)
		{
			numSpans = 3;
		}
		else if (arcObj->angle > Z_HALF_PI)
		{
			numSpans = 2;
		}

		double divide_angle = arcObj->angle / numSpans;
		for (int i = 0; i < numSpans; i++)
		{
			double s_angle = divide_angle * i;
			double m_angle = divide_angle * (i + 0.5);

			zPoint s_p = arcObj->oPlane.origin + arcObj->oPlane.xAxis * arcObj->radius * cos(s_angle) + arcObj->oPlane.yAxis * arcObj->radius * sin(s_angle);
			zVector m_v = arcObj->oPlane.xAxis * arcObj->radius * cos(m_angle) + arcObj->oPlane.yAxis * arcObj->radius * sin(m_angle);
			zPoint m_p = arcObj->oPlane.origin + m_v * (1/cos(divide_angle/2));
			arcObj->controlPoints.push_back(s_p);
			arcObj->controlPoints.push_back(m_p);
		}

		zPoint e_p = arcObj->oPlane.origin + arcObj->oPlane.xAxis * arcObj->radius * cos(arcObj->angle) + arcObj->oPlane.yAxis * arcObj->radius * sin(arcObj->angle);
		arcObj->controlPoints.push_back(e_p);
	}

	ZSPACE_INLINE bool zFnArc::isCircle()
	{
		if (arcObj->angle == Z_TWO_PI)
		{
			arcObj->isCircle = true;
		}
		else
		{
			arcObj->isCircle = false;
		}

		return arcObj->isCircle;
		//return false;
	}

	//--- GET METHODS 

	ZSPACE_INLINE double zFnArc::getAngle()
	{
		return arcObj->angle;
	}

	ZSPACE_INLINE double zFnArc::getRadius()
	{
		return arcObj->radius;
	}

	ZSPACE_INLINE zPoint* zFnArc::getRawControlPoints()
	{
		return &arcObj->controlPoints[0];
	}

	ZSPACE_INLINE zPoint* zFnArc::getRawArcPositions(int& numPoints)
	{
		numPoints = arcObj->arcPositions.size();
		return &arcObj->arcPositions[0];
	}

	ZSPACE_INLINE zPoint zFnArc::getPointAt(double t)
	{
		ON_3dPoint p = arcObj->on_arc.PointAt(t);
		return zPoint(p.x, p.y, p.z);
	}

	ZSPACE_INLINE zVector zFnArc::getTangentAt(double t)
	{
		ON_3dVector tan = arcObj->on_arc.TangentAt(t);
		return zVector(tan.x, tan.y, tan.z);
	}

	ZSPACE_INLINE void zFnArc::getPointTangentAt(double t, zPoint& p, zVector& tan)
	{
		p = getPointAt(t);
		tan = getTangentAt(t);
	}

	ZSPACE_INLINE zPoint zFnArc::getCenter()
	{
		return arcObj->oPlane.origin;
	}

	ZSPACE_INLINE double zFnArc::getLength()
	{
		return arcObj->on_arc.Length();
	}

	/*ZSPACE_INLINE zDomainDouble zFnArc::getDomain()
	{
		zDomainDouble out;
		nurbsCurveObj->curve.GetDomain(&out.min, &out.max);

		return out;
	}*/

	/*ZSPACE_INLINE zDoubleArray zFnArc::getKnotVector()
	{
		zDoubleArray out;
		out.assign(nurbsCurveObj->curve.KnotCount(), -1);
		
		for (int i = 0; i < nurbsCurveObj->curve.KnotCount(); i++)
		{
			out[i] = nurbsCurveObj->curve.m_knot[i];
		}		
		return out;
	}*/

	/*ZSPACE_INLINE int zFnArc::getDegree()
	{
		return nurbsCurveObj->degree;
	}*/

	ZSPACE_INLINE ON_Circle* zFnArc::getRawON_Arc()
	{
		return &arcObj->on_arc;
	}


	//---- TRANSFORM METHODS OVERRIDES

	ZSPACE_INLINE void zFnArc::setTransform(zTransform &inTransform, bool decompose, bool updatePositions)
	{
		if (updatePositions)
		{
			zTransformationMatrix to;
			to.setTransform(inTransform, decompose);

			zTransform transMat = arcObj->transformationMatrix.getToMatrix(to);
			transformObject(transMat);

			arcObj->transformationMatrix.setTransform(inTransform);

			// update pivot values of object transformation matrix
			zVector p = arcObj->transformationMatrix.getPivot();
			p = p * transMat;
			setPivot(p);
		}
		else
		{
			arcObj->transformationMatrix.setTransform(inTransform, decompose);

			zVector p = arcObj->transformationMatrix.getO();
			setPivot(p);
		}
	}

	ZSPACE_INLINE void zFnArc::setScale(zFloat4 &scale)
	{
		// get  inverse pivot translations
		zTransform invScalemat = arcObj->transformationMatrix.asInverseScaleTransformMatrix();

		// set scale values of object transformation matrix
		arcObj->transformationMatrix.setScale(scale);

		// get new scale transformation matrix
		zTransform scaleMat = arcObj->transformationMatrix.asScaleTransformMatrix();

		// compute total transformation
		zTransform transMat = invScalemat * scaleMat;

		// transform object
		transformObject(transMat);
	}

	ZSPACE_INLINE void zFnArc::setRotation(zFloat4 &rotation, bool appendRotations)
	{
		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = arcObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = arcObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = arcObj->transformationMatrix;
		to.setRotation(rotation, appendRotations);
		zTransform toMat = arcObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set rotation values of object transformation matrix
		arcObj->transformationMatrix.setRotation(rotation, appendRotations);
	}

	ZSPACE_INLINE void zFnArc::setTranslation(zVector &translation, bool appendTranslations)
	{
		// get vector as zDouble3
		zFloat4 t;
		translation.getComponents(t);

		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = arcObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = arcObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = arcObj->transformationMatrix;
		to.setTranslation(t, appendTranslations);
		zTransform toMat = arcObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set translation values of object transformation matrix
		arcObj->transformationMatrix.setTranslation(t, appendTranslations);;

		// update pivot values of object transformation matrix
		zVector p = arcObj->transformationMatrix.getPivot();
		p = p * transMat;
		setPivot(p);

	}

	ZSPACE_INLINE void zFnArc::setPivot(zVector &pivot)
	{
		// get vector as zDouble3
		zFloat4 p;
		pivot.getComponents(p);

		// set pivot values of object transformation matrix
		arcObj->transformationMatrix.setPivot(p);
	}

	ZSPACE_INLINE void zFnArc::getTransform(zTransform &transform)
	{
		transform = arcObj->transformationMatrix.asMatrix();
	}

	//---- PROTECTED OVERRIDE METHODS
	// 

	ZSPACE_INLINE void zFnArc::transformObject(zTransform &transform)
	{
		// transform control points
		for (int i = 0; i < arcObj->controlPoints.size(); i++)
		{
			arcObj->controlPoints[i] = arcObj->controlPoints[i] * transform;
		}

		// transform arc positions
		for (int i = 0; i < arcObj->arcPositions.size(); i++)
		{
			arcObj->arcPositions[i] = arcObj->arcPositions[i] * transform;
		}

		// transform plane
		arcObj->oPlane.origin = arcObj->oPlane.origin * transform;
		arcObj->oPlane.xAxis = arcObj->oPlane.xAxis * transform;
		arcObj->oPlane.yAxis = arcObj->oPlane.yAxis * transform;
		arcObj->oPlane.normal = arcObj->oPlane.normal * transform;
		arcObj->oPlane.on_plane = ON_Plane(ON_3dPoint(arcObj->oPlane.origin.x, arcObj->oPlane.origin.y, arcObj->oPlane.origin.z),
			ON_3dVector(arcObj->oPlane.xAxis.x, arcObj->oPlane.xAxis.y, arcObj->oPlane.xAxis.z),
			ON_3dVector(arcObj->oPlane.yAxis.x, arcObj->oPlane.yAxis.y, arcObj->oPlane.yAxis.z));

		// transform on_arc
		arcObj->on_arc = ON_Arc(arcObj->oPlane.on_plane, arcObj->radius, arcObj->angle);
	}


	//---- PRIVATE METHODS

	/*ZSPACE_INLINE bool zFnArc::ON_NurbsCurve_GetLength(const ON_NurbsCurve& curve, double* length, double fractional_tolerance, const ON_Interval* sub_domain)
	{
		return curve.GetLength(length, fractional_tolerance, sub_domain);
	}*/

	
}