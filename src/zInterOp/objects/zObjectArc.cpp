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


#include<zspace/zInterOp/objects/zObjectArc.h>
#include<zspace/zInterOp/objects/zObjectPlane.h>
#include<zspace/zInterOp/objects/zObjectNurbsCurve.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjectArc::zObjectArc()
	{

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
#endif		
		displayPlane = false;
		displayControlPoints = false;
		displayArc = true;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjectArc::~zObjectArc() {}

	//---- GET METHODS

	ZSPACE_INLINE int zObjectArc::getVBO_ControlPointId()
	{
		return VBO_ControlPointId;
	}

	ZSPACE_INLINE int zObjectArc::getVBO_CurvePointId()
	{
		return VBO_CurvePointId;
	}

	ZSPACE_INLINE int zObjectArc::getVBO_CurveColorId()
	{
		return VBO_CurveColorId;
	}

	ZSPACE_INLINE void zObjectArc::getRawRadius(double _radius)
	{
		_radius = radius;
	}

	ZSPACE_INLINE void zObjectArc::getRawAngleRadian(double _angle)
	{
		_angle = angle;
	}

	ZSPACE_INLINE void zObjectArc::getRawPlane(zObjectPlane _oPlane)
	{
		_oPlane = oPlane;
	}
}
