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


#include<headers/zInterOp/objects/zObjCurve.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjCurve::zObjCurve()
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

	ZSPACE_INLINE zObjCurve::~zObjCurve() {}

	//---- SET METHODS

	ZSPACE_INLINE void zObjCurve::setDisplayElements(bool _displayControlPoints, bool _displayCurve)
	{
		displayControlPoints = _displayControlPoints;
		displayCurve = _displayCurve;
	}

	ZSPACE_INLINE void zObjCurve::setDisplayPositions(zPointArray& _positions)
	{
		displayPositions = _positions;
		numDisplayPositions = _positions.size();
	}

	ZSPACE_INLINE void zObjCurve::setDisplayColor(zColor _displayColor)
	{
		displayColor = _displayColor;
	}

	ZSPACE_INLINE void zObjCurve::setDisplayWeight(double _displayWeight)
	{
		displayWeight = _displayWeight;
	}

	ZSPACE_INLINE void zObjCurve::setDisplayControlPoints(zPointArray& _displayControlPoints)
	{
		displayControlPointPositions = _displayControlPoints;
	}

	ZSPACE_INLINE void zObjCurve::setControlPointWeights(zDoubleArray& _controlPointWeights)
	{
		for (int i = 0; i < _controlPointWeights.size(); i++)
		{
			on_curve.SetWeight(i, _controlPointWeights[i]);
		}
	}

	ZSPACE_INLINE void zObjCurve::setDegree(int _degree)
	{
		degree = _degree;
	}

	ZSPACE_INLINE void zObjCurve::setPeriodic(bool _periodic)
	{
		periodic = _periodic;
	}

	//---- GET METHODS

	ZSPACE_INLINE void zObjCurve::getDisplayPositions(zPointArray& _positions)
	{
		_positions = displayPositions;
	}

	ZSPACE_INLINE int zObjCurve::getNumDisplayPositions()
	{
		return numDisplayPositions;
	}

	ZSPACE_INLINE int zObjCurve::getDegree()
	{
		return on_curve.Dimension();
	}

	ZSPACE_INLINE bool zObjCurve::isPeriodic()
	{
		return on_curve.IsPeriodic();
	}

	ZSPACE_INLINE int zObjCurve::getVBO_ControlPointId()
	{
		return VBO_ControlPointId;
	}

	ZSPACE_INLINE int zObjCurve::getVBO_CurvePointId()
	{
		return VBO_CurvePointId;
	}

	ZSPACE_INLINE int zObjCurve::getVBO_CurveColorId()
	{
		return VBO_CurveColorId;
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zObjCurve::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		coreUtils.getBounds(displayPositions, minBB, maxBB);
	}

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
	// Do Nothing
#else
	   
	ZSPACE_INLINE void zObjCurve::draw()
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
			displayUtils->drawCurve(&displayControlPointPositions[0], zColor(), 1, displayControlPointPositions.size(), periodic);
		}


		// draw edges
		if (displayCurve)
		{
			displayUtils->drawCurve(&displayPositions[0], displayColor, displayWeight, displayPositions.size(),periodic);
		}

	
	}

#endif // !ZSPACE_UNREAL_INTEROP
}