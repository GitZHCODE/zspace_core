// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>
//

#include<zspace/zCore/field/zField2D.h>

namespace zSpace
{
	//---- CONSTRUCTOR

	template <typename T>
	ZSPACE_INLINE zField2D<T>::zField2D()
		: n_X(0), n_Y(0), unit_X(0.0), unit_Y(0.0), valuesperVertex(true)
	{
		fieldValues.clear();
	}

	template <typename T>
	ZSPACE_INLINE zField2D<T>::zField2D(zVector _minBB, zVector _maxBB, int _n_X, int _n_Y)
		: n_X(_n_X), n_Y(_n_Y), minBB(_minBB), maxBB(_maxBB), valuesperVertex(true)
	{
		unit_X = (n_X > 1) ? (maxBB.x - minBB.x) / (n_X - 1) : (maxBB.x - minBB.x);
		unit_Y = (n_Y > 1) ? (maxBB.y - minBB.y) / (n_Y - 1) : (maxBB.y - minBB.y);

		fieldValues.assign(static_cast<std::size_t>(n_X) * static_cast<std::size_t>(n_Y), T());
	}

	template <typename T>
	ZSPACE_INLINE zField2D<T>::zField2D(double _unit_X, double _unit_Y, int _n_X, int _n_Y, zVector _minBB)
		: n_X(_n_X), n_Y(_n_Y), unit_X(_unit_X), unit_Y(_unit_Y), minBB(_minBB), valuesperVertex(true)
	{
		maxBB = _minBB + zVector(unit_X * n_X, unit_Y * n_Y, 0);

		fieldValues.assign(static_cast<std::size_t>(n_X) * static_cast<std::size_t>(n_Y), T());
	}

	//---- DESTRUCTOR

	template <typename T>
	ZSPACE_INLINE zField2D<T>::~zField2D() {}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
	// explicit instantiation
	template class zField2D<zVector>;

	template class zField2D<float>;

#endif
}
