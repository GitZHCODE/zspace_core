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

#if defined (ZSPACE_RHINO_INTEROP)

#include "zInterop/functionsets/zFnPlane.h"

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnPlane::zFnPlane()
	{
		fnType = zFnType::zPlaneFn;
		planeObj = nullptr;
	}

	ZSPACE_INLINE zFnPlane::zFnPlane(zObjPlane& _planeObj)
	{
		fnType = zFnType::zPlaneFn;
		planeObj = &_planeObj;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFnPlane::~zFnPlane() {}

	//---- OVERRIDE METHODS
	
	ZSPACE_INLINE zFnType zFnPlane::getType()
	{
		return zPlaneFn;
	}

	
	ZSPACE_INLINE void zFnPlane::from(string path, zFileType type, bool staticGeom)
	{
		if (type == zJSON)
		{
			json j;

			bool chk = json_read(path, j);
			if (chk) from(j, staticGeom);			
		}

		else throw std::invalid_argument(" error: invalid zFileType type");
	}

	ZSPACE_INLINE void zFnPlane::from(json& j, bool staticGeom)
	{
		//zUtilsJsonHE graphJSON;
		//
		//// read data to json 
		//

		//// Curve Attributes
		//vector<zDoubleArray> cPoints;		
		//readJSONAttribute(j, "NC_controlPoints", cPoints);

		//zDoubleArray cPointWeights;
		//readJSONAttribute(j, "NC_weights", cPointWeights);
		//

		//int cDegree;
		//readJSONAttribute(j, "NC_degree", cDegree);
		//nurbsCurveObj->degree = cDegree;

		//bool cPeriodic;
		//readJSONAttribute(j, "NC_periodic", cPeriodic);
		//nurbsCurveObj->periodic = cPeriodic;
		//
		////printf("\n vertexAttributes: %zi %zi", vertexAttributes.size(), vertexAttributes[0].size());
		//nurbsCurveObj->controlPoints.clear();
		//nurbsCurveObj->displayPositions.clear();
		//nurbsCurveObj->controlPointWeights.clear();

		//ON_3dPointArray points;
		//
		//for (int i = 0; i < cPoints.size(); i++)
		//{
		//	nurbsCurveObj->controlPointWeights.push_back(cPointWeights[i]);
		//	points.Append(ON_3dPoint(cPoints[i][0], cPoints[i][1], cPoints[i][2]));			
		//}

		//const int order = cDegree + 1;
		//	
		//bool check =	(cPeriodic) ? nurbsCurveObj->curve.CreatePeriodicUniformNurbs(3, order, points.Count(), points) : nurbsCurveObj->curve.CreateClampedUniformNurbs(3, order, points.Count(), points);
		//printf("\n curve created %s ", (check) ? "True" : "False");

		//// store control points
		//for (int i = 0; i < points.Count(); i++)
		//{
		//	//nurbsCurveObj->curve.SetCV(i, points[i]);
		//	nurbsCurveObj->curve.SetWeight(i, cPointWeights[i]);			
		//	
		//	zPoint cP(points[i].x, points[i].y, points[i].z);
		//	nurbsCurveObj->controlPoints.push_back(cP);			
		//}
		//
		//// normalise curve
		//nurbsCurveObj->curve.SetDomain(0.0, 1.0);
		//
		//// display points 
		//setDisplayNumPoints();
		//	
		//
	}

	ZSPACE_INLINE void zFnPlane::to(string path, zFileType type)
	{
		if (type == zJSON)
		{
			json j;
			to(j);
			bool chk = json_write(path, j);
			//toJSON(path);
		}

		else throw std::invalid_argument(" error: invalid zFileType type");
	}

	ZSPACE_INLINE void zFnPlane::to(json& j)
	{
		//vector<zDoubleArray> cPoints;
		//cPoints.assign(nurbsCurveObj->controlPoints.size(), zDoubleArray());

		//zDoubleArray cPointWeights;
		//cPointWeights.assign(nurbsCurveObj->controlPoints.size(), -1);

		//for (int i = 0; i < nurbsCurveObj->controlPoints.size(); i++)
		//{
		//	cPoints[i].push_back(nurbsCurveObj->controlPoints[i].x);
		//	cPoints[i].push_back(nurbsCurveObj->controlPoints[i].y);
		//	cPoints[i].push_back(nurbsCurveObj->controlPoints[i].z);

		//	cPointWeights[i] = nurbsCurveObj->controlPointWeights[i];
		//}

		//int cDegree = nurbsCurveObj->degree;
		//bool cPeriodic = nurbsCurveObj->periodic;		

		//zDomainDouble domain = getDomain();
		//zDoubleArray cDomain = { domain.min, domain.max};

		//zDoubleArray cKnots = getKnotVector();


		//// Json file 
		//coreUtils.writeJSONAttribute(j, "NC_controlPoints", cPoints);
		//coreUtils.writeJSONAttribute(j, "NC_weights", cPointWeights);
		//coreUtils.writeJSONAttribute(j, "NC_degree", cDegree);
		//coreUtils.writeJSONAttribute(j, "NC_domain", cDomain);
		//coreUtils.writeJSONAttribute(j, "NC_knots", cKnots);
		//coreUtils.writeJSONAttribute(j, "NC_periodic", cPeriodic);
		//
	}

#if defined ZSPACE_USD_INTEROP

	ZSPACE_INLINE void zFnPlane::from(UsdPrim& usd, bool staticGeom)
	{
	}

	ZSPACE_INLINE void zFnPlane::to(UsdPrim& usd)
	{
	}

#endif

	ZSPACE_INLINE void zFnPlane::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		planeObj->getBounds(minBB, maxBB);
	}

	ZSPACE_INLINE void zFnPlane::clear()
	{

	}

	//---- CREATE METHODS

	ZSPACE_INLINE bool zFnPlane::create(zPoint origin, zVector xAxis, zVector yAxis)
	{
		bool success = false;

		xAxis.normalize();
		yAxis.normalize();

		if (xAxis.length() == 0 || yAxis.length() == 0)
		{
			return success = false;
		}
		else if ((xAxis - yAxis).length() == 0 || (xAxis + yAxis).length() == 0)
		{
			return success = false;
		}
		else
		{
			planeObj->origin = origin;
			planeObj->xAxis = xAxis;	

			planeObj->normal = xAxis ^ yAxis;
			planeObj->normal.normalize();

			planeObj->yAxis = (planeObj->normal) ^ xAxis;
			planeObj->yAxis.normalize();

			planeObj->on_plane = ON_Plane(ON_3dPoint(origin.x, origin.y, origin.z), 
				ON_3dVector(xAxis.x, xAxis.y, xAxis.z), 
				ON_3dVector(yAxis.x, yAxis.y, yAxis.z));

			success = true;
		}
		return success;
	}

	ZSPACE_INLINE bool zFnPlane::createFromMatrix(Eigen::Matrix4f& _plane)
	{
		bool success = false;

		zPoint origin(_plane(0, 3), _plane(1, 3), _plane(2, 3));
		zVector xAxis(_plane(0, 0), _plane(1, 0), _plane(2, 0));
		zVector yAxis(_plane(0, 1), _plane(1, 1), _plane(2, 1));

		success = create(origin, xAxis, yAxis);

		return success;
	}

	ZSPACE_INLINE bool zFnPlane::createFromNormal(zPoint origin, zVector normal, zVector yUp)
	{
		bool success = false;

		normal.normalize();
		yUp.normalize();
		
		if (normal.length() == 0 || yUp.length() == 0)
		{
			return success = false;
		}
		else if ((normal - yUp).length() == 0 || (normal + yUp).length() == 0)
		{
			return success = false;
		}
		else
		{
			planeObj->origin = origin;
			planeObj->normal = normal;

			planeObj->xAxis = normal ^ yUp;
			planeObj->xAxis.normalize();

			planeObj->yAxis = (planeObj->normal) ^ (planeObj->xAxis);
			planeObj->yAxis.normalize();

			planeObj->on_plane = ON_Plane(ON_3dPoint(origin.x, origin.y, origin.z), 
				ON_3dVector(planeObj->xAxis.x, planeObj->xAxis.y, planeObj->xAxis.z),
				ON_3dVector(planeObj->yAxis.x, planeObj->yAxis.y, planeObj->yAxis.z));

			success = true;
		}
		return success;
	}

	ZSPACE_INLINE bool zFnPlane::createFromPoints(zPoint p0, zPoint p1, zPoint p2)
	{
		bool success = false;

		zVector xAxis = p1 - p0;
		zVector yAxis = p2 - p0;

		success = create(p0, xAxis, yAxis);

		return success;
	}
	
	ZSPACE_INLINE void zFnPlane::intersect()
	{
		//ON_3dPoint o = coreUtils.
	}

	//--- SET METHODS 

	ZSPACE_INLINE void zFnPlane::setDisplayColor(zColor xCol, zColor yCol, zColor zCol)
	{
		planeObj->displayColor.clear();
		planeObj->displayColor.push_back(xCol);
		planeObj->displayColor.push_back(yCol);
		planeObj->displayColor.push_back(zCol);
	}

	ZSPACE_INLINE void zFnPlane::setDisplayWeight(double _wt)
	{
		planeObj->displayWeight = _wt;
	}

	ZSPACE_INLINE void zFnPlane::setOrigin(zPoint origin)
	{
		planeObj->origin = origin;
	}

	ZSPACE_INLINE void zFnPlane::setAxis(zVector xAxis, zVector yAxis, zVector normal)
	{
		planeObj->xAxis = xAxis;
		planeObj->yAxis = yAxis;
		planeObj->normal = normal;
	}


	//--- GET METHODS 


	ZSPACE_INLINE zPoint zFnPlane::getOrigin()
	{
		return planeObj->origin;
	}

	ZSPACE_INLINE zVector zFnPlane::getXAxis()
	{
		return planeObj->xAxis;
	}

	ZSPACE_INLINE zVector zFnPlane::getYAxis()
	{
		return planeObj->yAxis;
	}

	ZSPACE_INLINE zVector zFnPlane::getNormal()
	{
		return planeObj->normal;
	}

	ZSPACE_INLINE ON_Plane* zFnPlane::getRawON_Plane()
	{
		return &planeObj->on_plane;
	}


	//---- TRANSFORM METHODS OVERRIDES

	ZSPACE_INLINE void zFnPlane::setTransform(zTransform &inTransform, bool decompose, bool updatePositions)
	{
		if (updatePositions)
		{
			zTransformationMatrix to;
			to.setTransform(inTransform, decompose);

			zTransform transMat = planeObj->transformationMatrix.getToMatrix(to);
			transformObject(transMat);

			planeObj->transformationMatrix.setTransform(inTransform);

			// update pivot values of object transformation matrix
			zVector p = planeObj->transformationMatrix.getPivot();
			p = p * transMat;
			setPivot(p);

		}
		else
		{
			planeObj->transformationMatrix.setTransform(inTransform, decompose);

			zVector p = planeObj->transformationMatrix.getO();
			setPivot(p);

		}

	}

	ZSPACE_INLINE void zFnPlane::setScale(zFloat4 &scale)
	{
		// get  inverse pivot translations
		zTransform invScalemat = planeObj->transformationMatrix.asInverseScaleTransformMatrix();

		// set scale values of object transformation matrix
		planeObj->transformationMatrix.setScale(scale);

		// get new scale transformation matrix
		zTransform scaleMat = planeObj->transformationMatrix.asScaleTransformMatrix();

		// compute total transformation
		zTransform transMat = invScalemat * scaleMat;

		// transform object
		transformObject(transMat);
	}

	ZSPACE_INLINE void zFnPlane::setRotation(zFloat4 &rotation, bool appendRotations)
	{
		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = planeObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = planeObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = planeObj->transformationMatrix;
		to.setRotation(rotation, appendRotations);
		zTransform toMat = planeObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set rotation values of object transformation matrix
		planeObj->transformationMatrix.setRotation(rotation, appendRotations);;
	}

	ZSPACE_INLINE void zFnPlane::setTranslation(zVector &translation, bool appendTranslations)
	{
		// get vector as zDouble3
		zFloat4 t;
		translation.getComponents(t);

		// get pivot translation and inverse pivot translations
		zTransform pivotTransMat = planeObj->transformationMatrix.asPivotTranslationMatrix();
		zTransform invPivotTransMat = planeObj->transformationMatrix.asInversePivotTranslationMatrix();

		// get plane to plane transformation
		zTransformationMatrix to = planeObj->transformationMatrix;
		to.setTranslation(t, appendTranslations);
		zTransform toMat = planeObj->transformationMatrix.getToMatrix(to);

		// compute total transformation
		zTransform transMat = invPivotTransMat * toMat * pivotTransMat;

		// transform object
		transformObject(transMat);

		// set translation values of object transformation matrix
		planeObj->transformationMatrix.setTranslation(t, appendTranslations);;

		// update pivot values of object transformation matrix
		zVector p = planeObj->transformationMatrix.getPivot();
		p = p * transMat;
		setPivot(p);

	}

	ZSPACE_INLINE void zFnPlane::setPivot(zVector &pivot)
	{
		// get vector as zDouble3
		zFloat4 p;
		pivot.getComponents(p);

		// set pivot values of object transformation matrix
		planeObj->transformationMatrix.setPivot(p);
	}

	ZSPACE_INLINE void zFnPlane::getTransform(zTransform &transform)
	{
		transform = planeObj->transformationMatrix.asMatrix();
	}		

	//---- PROTECTED OVERRIDE METHODS
	ZSPACE_INLINE void zFnPlane::transformObject(zTransform &transform)
	{
		planeObj->origin = planeObj->origin * transform;
		planeObj->xAxis = planeObj->xAxis * transform;
		planeObj->yAxis = planeObj->yAxis * transform;
		planeObj->normal = planeObj->normal * transform;
		planeObj->on_plane = ON_Plane(ON_3dPoint(planeObj->origin.x, planeObj->origin.y, planeObj->origin.z),
						ON_3dVector(planeObj->xAxis.x, planeObj->xAxis.y, planeObj->xAxis.z),
						ON_3dVector(planeObj->yAxis.x, planeObj->yAxis.y, planeObj->yAxis.z));

	}

	// 
	//---- PRIVATE METHODS
	
}
#endif