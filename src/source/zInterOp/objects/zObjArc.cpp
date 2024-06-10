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

#if defined (ZSPACE_RHINO_INTEROP)

#include "zInterOp/objects/zObjArc.h"
#include "zInterOp/objects/zObjPlane.h"
#include "zInterOp/objects/zObjNurbsCurve.h"

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjArc::zObjArc()
	{

#if defined (ZSPACE_VIEWER) || defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		displayUtils = nullptr;
#endif		
		displayPlane = false;
		displayControlPoints = false;
		displayArc = true;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjArc::~zObjArc() {}

	//---- SET METHODS

	ZSPACE_INLINE void zObjArc::setDisplayElements(bool _displayArc, bool _displayControlPoints, bool _displayPlane)
	{		
		displayArc = _displayArc;
		displayControlPoints = _displayControlPoints;
		displayPlane = _displayPlane;
	}

	ZSPACE_INLINE void zObjArc::setArcDisplayColor(zColor _arcDisplayColor)
	{
		arcDisplayColor = _arcDisplayColor;
	}

	ZSPACE_INLINE void zObjArc::setControlPointsColor(zColor _controlPointsColor)
	{
		controlPointsColor = _controlPointsColor;
	}

	ZSPACE_INLINE void zObjArc::setControlPolyColor(zColor _controlPolyColor)
	{
		controlPolyColor = _controlPolyColor;
	}

	ZSPACE_INLINE void zObjArc::setControlPointsWeight(double _controlPointsWeight)
	{
		controlPointsWeight = _controlPointsWeight;
	}

	ZSPACE_INLINE void zObjArc::setControlPolyWeight(double _controlPolyWeight)
	{
		controlPolyWeight = _controlPolyWeight;
	}

	//---- GET METHODS

	ZSPACE_INLINE int zObjArc::getVBO_ControlPointId()
	{
		return VBO_ControlPointId;
	}

	ZSPACE_INLINE int zObjArc::getVBO_CurvePointId()
	{
		return VBO_CurvePointId;
	}

	ZSPACE_INLINE int zObjArc::getVBO_CurveColorId()
	{
		return VBO_CurveColorId;
	}

	ZSPACE_INLINE void zObjArc::getRawRadius(double _radius)
	{
		_radius = radius;
	}

	ZSPACE_INLINE void zObjArc::getRawAngleRadian(double _angle)
	{
		_angle = angle;
	}

	ZSPACE_INLINE void zObjArc::getRawPlane(zObjPlane _oPlane)
	{
		_oPlane = oPlane;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zObjArc::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		coreUtils.getBounds(arcPositions, minBB, maxBB);
	}

#if defined (ZSPACE_VIEWER) || defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
	// Do Nothing
#else
	   
	ZSPACE_INLINE void zObjArc::draw()
	{
		if (displayObject)
		{
			drawArc();
		}

		if (displayObjectTransform)
		{
			displayUtils->drawTransform(transformationMatrix);
		}
	}

	//---- DISPLAY BUFFER METHODS

	ZSPACE_INLINE void zObjArc::appendToBuffer()
	{
		
	}
	
	//---- PROTECTED DISPLAY METHODS
	ZSPACE_INLINE void zObjArc::drawArc()
	{
		if (displayArc)
		{
			displayUtils->drawCurve(&arcPositions[0], arcDisplayColor, arcDisplayWeight, arcPositions.size(), false);
		}

		if (displayControlPoints)
		{
			displayUtils->drawPoints(&controlPoints[0], controlPointsColor, controlPointsWeight, controlPoints.size());
			displayUtils->drawCurve(&controlPoints[0], controlPolyColor, controlPolyWeight, controlPoints.size(), false);
		}

		if (displayPlane)
		{
			oPlane.draw();
		}

	}

#endif // !ZSPACE_UNREAL_INTEROP
}
#endif //!ZSPACE_RHINO_INTEROP