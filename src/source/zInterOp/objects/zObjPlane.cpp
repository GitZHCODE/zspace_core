// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2023 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Ling Mao <Ling.Mao@zaha-hadid.com>
//


#include "zInterOp/objects/zObjPlane.h"

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjPlane::zObjPlane()
	{

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		//displayUtils = nullptr;
#endif

		displayRectangle = false;
		displayAxis = true;

		displayColor.push_back(zColor(1.0, 0.0, 0.0, 1.0));
		displayColor.push_back(zColor(0.0, 1.0, 0.0, 1.0));
		displayColor.push_back(zColor(0.0, 0.0, 1.0, 1.0));
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjPlane::~zObjPlane() {}

	//---- SET METHODS

	ZSPACE_INLINE void zObjPlane::setDisplayElements(bool _displayAxis, bool _displayRectangle, double _displayAxisScale, double _displayRectangleScale)
	{
		displayAxis = _displayAxis;
		displayRectangle = _displayRectangle;
		displayAxisScale = _displayAxisScale;
		displayRectangleScale = _displayRectangleScale;		
	}

	ZSPACE_INLINE void zObjPlane::setDisplayColor(zColorArray _displayColorArray)
	{
		displayColor.clear();
		displayColor = _displayColorArray;
	}


	//---- GET METHODS
	
	ZSPACE_INLINE zPlane zObjPlane::getPlaneMatrix()
	{
		zPlane planeMatrix;
		planeMatrix(0,0)= xAxis.x;
		planeMatrix(0,1)= xAxis.y;
		planeMatrix(0,2)= xAxis.z;
		planeMatrix(0,3)= origin.x;
		planeMatrix(1,0)= yAxis.x;
		planeMatrix(1,1)= yAxis.y;
		planeMatrix(1,2)= yAxis.z;
		planeMatrix(1,3)= origin.y;
		planeMatrix(2,0)= normal.x;
		planeMatrix(2,1)= normal.y;
		planeMatrix(2,2)= normal.z;
		planeMatrix(2,3)= origin.z;
		planeMatrix(3,0)= 0;
		planeMatrix(3,1)= 0;
		planeMatrix(3,2)= 0;
		planeMatrix(3,3)= 1;
		return planeMatrix;
	}
	
	ZSPACE_INLINE zDoubleArray zObjPlane::getDoubleArray()
	{
		zDoubleArray planeArray;
		planeArray.push_back(xAxis.x);
		planeArray.push_back(xAxis.y);
		planeArray.push_back(xAxis.z);
		planeArray.push_back(origin.x);
		planeArray.push_back(yAxis.x);
		planeArray.push_back(yAxis.y);
		planeArray.push_back(yAxis.z);
		planeArray.push_back(origin.y);
		planeArray.push_back(normal.x);
		planeArray.push_back(normal.y);
		planeArray.push_back(normal.z);
		planeArray.push_back(origin.z);
		planeArray.push_back(0);
		planeArray.push_back(0);
		planeArray.push_back(0);
		planeArray.push_back(1);
		return planeArray;
	}

	ZSPACE_INLINE void zObjPlane::getRawOrigin(zPoint& _origin)
	{
		_origin = origin;
	}

	ZSPACE_INLINE void zObjPlane::getRawXAxis(zVector& _xAxis)
	{
		_xAxis = xAxis;
	}

	ZSPACE_INLINE void zObjPlane::getRawYAxis(zVector& _yAxis)
	{
		_yAxis = yAxis;
	}

	ZSPACE_INLINE void zObjPlane::getRawNormal(zVector& _Normal)
	{
		_Normal = normal;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zObjPlane::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		//coreUtils.getBounds(displayPositions, minBB, maxBB);
	}

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
	// Do Nothing
#else
	   
	//ZSPACE_INLINE void zObjPlane::draw()
	//{
	//	if (displayObject)
	//	{
	//		drawPlane();
	//	}

	//	if (displayObjectTransform)
	//	{
	//		displayUtils->drawTransform(transformationMatrix);
	//	}
	//}

	//---- DISPLAY BUFFER METHODS

	//ZSPACE_INLINE void zObjPlane::appendToBuffer()
	//{
	//	
	//}
	
	//---- PROTECTED DISPLAY METHODS
	//ZSPACE_INLINE void zObjPlane::drawPlane()
	//{
	//	if(displayAxis)
	//	{
	//		zPoint p0 = origin + xAxis * displayAxisScale;
	//		zPoint p1 = origin + yAxis * displayAxisScale;
	//		zPoint p2 = origin + normal * displayAxisScale;

	//		displayUtils->drawLine(origin, p0, displayColor[0], displayWeight);
	//		displayUtils->drawLine(origin, p1, displayColor[1], displayWeight);
	//		displayUtils->drawLine(origin, p2, displayColor[2], displayWeight);
	//	}

	//	// draw vertex
	//	if (displayRectangle)
	//	{
	//		int divisionNum = 10;
	//		zPointArray pU1;
	//		zPointArray pU2;
	//		zPointArray pV1;
	//		zPointArray pV2;

	//		zPoint p0 = origin + (xAxis + yAxis) * displayRectangleScale;
	//		zPoint p1 = origin + (xAxis - yAxis) * displayRectangleScale;
	//		zPoint p2 = origin - (xAxis + yAxis) * displayRectangleScale;
	//		zPoint p3 = origin - (xAxis - yAxis) * displayRectangleScale;

	//		for (int i = 0; i <= divisionNum; i++)
	//		{
	//			pU1.push_back(p0 * i / divisionNum + p1 * (divisionNum - i) / divisionNum);
	//			pU2.push_back(p3 * i / divisionNum + p2 * (divisionNum - i) / divisionNum);
	//			pV1.push_back(p1 * i / divisionNum + p2 * (divisionNum - i) / divisionNum);
	//			pV2.push_back(p0 * i / divisionNum + p3 * (divisionNum - i) / divisionNum);
	//		}
	//				
	//		for (int i = 0; i <= divisionNum; i++)
	//		{
	//			displayUtils->drawLine(pU1[i], pU2[i], zColor(), 1);
	//			displayUtils->drawLine(pV1[i], pV2[i], zColor(), 1);
	//		}
	//		
	//	}
	//}

#endif // !ZSPACE_UNREAL_INTEROP
}