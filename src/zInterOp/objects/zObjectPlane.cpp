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


#include<zspace/zInterOp/objects/zObjectPlane.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjectPlane::zObjectPlane()
	{

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
#endif

		displayRectangle = false;
		displayAxis = true;

		displayColor.push_back(zColor(1.0, 0.0, 0.0, 1.0));
		displayColor.push_back(zColor(0.0, 1.0, 0.0, 1.0));
		displayColor.push_back(zColor(0.0, 0.0, 1.0, 1.0));
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjectPlane::~zObjectPlane() {}

	//---- GET METHODS
	
	ZSPACE_INLINE zPlane zObjectPlane::getPlaneMatrix()
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
	
	ZSPACE_INLINE zDoubleArray zObjectPlane::getDoubleArray()
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

	ZSPACE_INLINE void zObjectPlane::getRawOrigin(zPoint& _origin)
	{
		_origin = origin;
	}

	ZSPACE_INLINE void zObjectPlane::getRawXAxis(zVector& _xAxis)
	{
		_xAxis = xAxis;
	}

	ZSPACE_INLINE void zObjectPlane::getRawYAxis(zVector& _yAxis)
	{
		_yAxis = yAxis;
	}

	ZSPACE_INLINE void zObjectPlane::getRawNormal(zVector& _Normal)
	{
		_Normal = normal;
	}
}
