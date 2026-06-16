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


#include<zspace/zInterOp/objects/zObjectNurbsCurve.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjectNurbsCurve::zObjectNurbsCurve()
	{

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
		// Do Nothing
#else
#endif

		displayControlPoints = false;
		displayCurve = true;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjectNurbsCurve::~zObjectNurbsCurve() {}

	ZSPACE_INLINE void zObjectNurbsCurve::setControlPointWeights(zDoubleArray& _controlPointWeights)
	{
		for (int i = 0; i < _controlPointWeights.size(); i++)
		{
			curve.SetWeight(i, _controlPointWeights[i]);
		}
	}

	ZSPACE_INLINE void zObjectNurbsCurve::setDegree(int _degree)
	{
		degree = _degree;
	}

	ZSPACE_INLINE void zObjectNurbsCurve::setPeriodic(bool _periodic)
	{
		periodic = _periodic;
	}

	//---- GET METHODS

	ZSPACE_INLINE void zObjectNurbsCurve::getDisplayPositions(zPointArray& _positions)
	{
		_positions = displayPositions;
	}

	ZSPACE_INLINE int zObjectNurbsCurve::getNumDisplayPositions()
	{
		return numDisplayPositions;
	}

	ZSPACE_INLINE int zObjectNurbsCurve::getDegree()
	{
		return curve.Dimension();
	}

	ZSPACE_INLINE bool zObjectNurbsCurve::isPeriodic()
	{
		return curve.IsPeriodic();
	}

	ZSPACE_INLINE int zObjectNurbsCurve::getVBO_ControlPointId()
	{
		return VBO_ControlPointId;
	}

	ZSPACE_INLINE int zObjectNurbsCurve::getVBO_CurvePointId()
	{
		return VBO_CurvePointId;
	}

	ZSPACE_INLINE int zObjectNurbsCurve::getVBO_CurveColorId()
	{
		return VBO_CurveColorId;
	}
}
