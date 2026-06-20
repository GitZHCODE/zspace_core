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


#include<zspace/zInterOp/functionsets/zFnPlane.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zFnPlane::zFnPlane()
	{
		fnType = zFnType::zPlaneFn;
		planeObj = nullptr;
	}

	ZSPACE_INLINE zFnPlane::zFnPlane(zObjectPlane& _planeObj)
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

	
				#if defined ZSPACE_USD_INTEROP

		#endif

	ZSPACE_INLINE void zFnPlane::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		minBB = planeObj->origin;
		maxBB = planeObj->origin;
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
