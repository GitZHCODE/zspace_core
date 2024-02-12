// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2023 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//


#include<headers/zInterOp/objects/zObjNurbsCurve.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjNurbsCurve::zObjNurbsCurve()
	{

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		displayUtils = nullptr;
#endif

		displayControlPoints = false;
		displayCurve = true;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjNurbsCurve::~zObjNurbsCurve() {}

	//---- SET METHODS

	ZSPACE_INLINE void zObjNurbsCurve::setDisplayElements(bool _displayControlPoints, bool _displayCurve)
	{
		displayControlPoints = _displayControlPoints;
		displayCurve = _displayCurve;
	}

	ZSPACE_INLINE void zObjNurbsCurve::setDisplayPositions(zPointArray& _positions)
	{
		displayPositions = _positions;
		numDisplayPositions = _positions.size();
	}

	ZSPACE_INLINE void zObjNurbsCurve::setDisplayColor(zColor _displayColor)
	{
		displayColor = _displayColor;
	}

	ZSPACE_INLINE void zObjNurbsCurve::setDisplayWeight(double _displayWeight)
	{
		displayWeight = _displayWeight;
	}

	ZSPACE_INLINE void zObjNurbsCurve::setDisplayControlPoints(zPointArray& _displayControlPoints)
	{
		displayControlPointPositions = _displayControlPoints;
	}

	//---- GET METHODS

	ZSPACE_INLINE void zObjNurbsCurve::getDisplayPositions(zPointArray& _positions)
	{
		_positions = displayPositions;
	}

	ZSPACE_INLINE int zObjNurbsCurve::getNumDisplayPositions()
	{
		return numDisplayPositions;
	}

	ZSPACE_INLINE int zObjNurbsCurve::getVBO_ControlPointId()
	{
		return VBO_ControlPointId;
	}

	ZSPACE_INLINE int zObjNurbsCurve::getVBO_CurvePointId()
	{
		return VBO_CurvePointId;
	}

	ZSPACE_INLINE int zObjNurbsCurve::getVBO_CurveColorId()
	{
		return VBO_CurveColorId;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zObjNurbsCurve::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		coreUtils.getBounds(displayPositions, minBB, maxBB);
	}

	//---- OPERATOR
	ZSPACE_INLINE zObjNurbsCurve& zObjNurbsCurve::operator=(const ON_NurbsCurve& rhs)
	{
		ON_NurbsCurve::operator=(rhs);
		return *this;
	}

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
	// Do Nothing
#else
	   
	ZSPACE_INLINE void zObjNurbsCurve::draw()
	{
		if (displayObject)
		{
			drawNurbsCurve();
		}

		if (displayObjectTransform)
		{
			displayUtils->drawTransform(transformationMatrix);
		}
	}

	//---- DISPLAY BUFFER METHODS

	ZSPACE_INLINE void zObjNurbsCurve::appendToBuffer()
	{
		
	}
	
	//---- PROTECTED DISPLAY METHODS

	ZSPACE_INLINE void zObjNurbsCurve::drawNurbsCurve()
	{
		// draw vertex
		if (displayControlPoints)
		{
			displayUtils->drawPoints(&displayControlPointPositions[0], zColor(), 3, displayControlPointPositions.size());
			displayUtils->drawCurve(&displayControlPointPositions[0], zColor(), 1, displayControlPointPositions.size(), this->IsPeriodic());
		}


		// draw edges
		if (displayCurve)
		{
			displayUtils->drawCurve(&displayPositions[0], displayColor, displayWeight, displayPositions.size(), this->IsPeriodic());
		}

	
	}

#endif // !ZSPACE_UNREAL_INTEROP
}