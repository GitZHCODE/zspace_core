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
		displayUtils = nullptr;
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

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zObjPlane::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		//coreUtils.getBounds(displayPositions, minBB, maxBB);
	}

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
	// Do Nothing
#else
	   
	ZSPACE_INLINE void zObjPlane::draw()
	{
		if (displayObject)
		{
			drawPlane();
		}

		if (displayObjectTransform)
		{
			displayUtils->drawTransform(transformationMatrix);
		}
	}

	//---- DISPLAY BUFFER METHODS

	ZSPACE_INLINE void zObjPlane::appendToBuffer()
	{
		
	}
	
	//---- PROTECTED DISPLAY METHODS
	ZSPACE_INLINE void zObjPlane::drawPlane()
	{
		if(displayAxis)
		{
			zPoint p0 = origin + xAxis * displayAxisScale;
			zPoint p1 = origin + yAxis * displayAxisScale;
			zPoint p2 = origin + normal * displayAxisScale;

			displayUtils->drawLine(origin, p0, displayColor[0], displayWeight);
			displayUtils->drawLine(origin, p1, displayColor[1], displayWeight);
			displayUtils->drawLine(origin, p2, displayColor[2], displayWeight);
		}

		// draw vertex
		if (displayRectangle)
		{
			zPoint p0 = origin + (xAxis + yAxis) * displayAxisScale;
			zPoint p1 = origin + (xAxis - yAxis) * displayAxisScale;
			zPoint p2 = origin - (xAxis + yAxis) * displayAxisScale;
			zPoint p3 = origin - (xAxis - yAxis) * displayAxisScale;

			displayUtils->drawLine(p0, p1, zColor(), 1);
			displayUtils->drawLine(p1, p2, zColor(), 1);
			displayUtils->drawLine(p2, p3, zColor(), 1);
			displayUtils->drawLine(p3, p0, zColor(), 1);
		}
	}

#endif // !ZSPACE_UNREAL_INTEROP
}