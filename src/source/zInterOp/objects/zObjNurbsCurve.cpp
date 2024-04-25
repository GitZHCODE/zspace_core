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

#if defined (ZSPACE_RHINO_INTEROP)

#include "zInterOp/objects/zObjNurbsCurve.h"

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjNurbsCurve::zObjNurbsCurve()
	{

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
		//displayUtils = nullptr;
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

	ZSPACE_INLINE void zObjNurbsCurve::setControlPointWeights(zDoubleArray& _controlPointWeights)
	{
		for (int i = 0; i < _controlPointWeights.size(); i++)
		{
			curve.SetWeight(i, _controlPointWeights[i]);
		}
	}

	ZSPACE_INLINE void zObjNurbsCurve::setDegree(int _degree)
	{
		degree = _degree;
	}

	ZSPACE_INLINE void zObjNurbsCurve::setPeriodic(bool _periodic)
	{
		periodic = _periodic;
	}

	//---- GET METHODS

	ZSPACE_INLINE zPoint& zObjNurbsCurve::GetControlPoint(int index)
	{
		return displayControlPointPositions[index];
	}

	ZSPACE_INLINE void zObjNurbsCurve::getDisplayPositions(zPointArray& _positions)
	{
		_positions = displayPositions;
	}

	ZSPACE_INLINE int zObjNurbsCurve::getNumDisplayPositions()
	{
		return numDisplayPositions;
	}

	ZSPACE_INLINE int zObjNurbsCurve::getDegree()
	{
		return curve.Dimension();
	}

	ZSPACE_INLINE bool zObjNurbsCurve::isPeriodic()
	{
		return curve.IsPeriodic();
	}

	//---- OVERRIDE METHODS

	ZSPACE_INLINE void zObjNurbsCurve::getBounds(zPoint &minBB, zPoint &maxBB)
	{
		coreUtils.getBounds(displayPositions, minBB, maxBB);
	}

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
	// Do Nothing
#else
	   
	//ZSPACE_INLINE void zObjNurbsCurve::draw()
	//{
	//	if (displayObject)
	//	{
	//		drawNurbsCurve();
	//	}

	//	if (displayObjectTransform)
	//	{
	//		displayUtils->drawTransform(transformationMatrix);
	//	}
	//}

	//---- DISPLAY BUFFER METHODS

	//ZSPACE_INLINE void zObjNurbsCurve::appendToBuffer()
	//{
	//	
	//}
	
	//---- PROTECTED DISPLAY METHODS

	//ZSPACE_INLINE void zObjNurbsCurve::drawNurbsCurve()
	//{
	//	// draw vertex
	//	if (displayControlPoints)
	//	{
	//		displayUtils->drawPoints(&displayControlPointPositions[0], zColor(), 3, displayControlPointPositions.size());
	//		displayUtils->drawCurve(&displayControlPointPositions[0], zColor(), 1, displayControlPointPositions.size(), periodic);
	//	}


	//	// draw edges
	//	if (displayCurve)
	//	{
	//		displayUtils->drawCurve(&displayPositions[0], displayColor, displayWeight, displayPositions.size(),periodic);
	//	}

	//
	//}

#endif // !ZSPACE_UNREAL_INTEROP
}
#endif //!ZSPACE_RHINO_INTEROP