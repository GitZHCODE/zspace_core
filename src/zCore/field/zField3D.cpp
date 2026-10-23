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

#include<zspace/zCore/field/zField3D.h>

namespace zSpace
{

	//---- CONSTRUCTOR

	template <typename T>
	ZSPACE_INLINE zField3D<T>::zField3D()
		: n_X(0), n_Y(0), n_Z(0), unit_X(0.0), unit_Y(0.0), unit_Z(0.0)
	{
		fieldValues.clear();
	}

	template <typename T>
	ZSPACE_INLINE zField3D<T>::zField3D(zVector _minBB, zVector _maxBB, int _n_X, int _n_Y, int _n_Z)
		: n_X(_n_X), n_Y(_n_Y), n_Z(_n_Z), minBB(_minBB), maxBB(_maxBB)
	{
		unit_X = (n_X > 0) ? (maxBB.x - minBB.x) / n_X : 0.0;
		unit_Y = (n_Y > 0) ? (maxBB.y - minBB.y) / n_Y : 0.0;
		unit_Z = (n_Z > 0) ? (maxBB.z - minBB.z) / n_Z : 0.0;

		fieldValues.assign(
			static_cast<std::size_t>(n_X) * static_cast<std::size_t>(n_Y) * static_cast<std::size_t>(n_Z),
			T());
	}

	template <typename T>
	ZSPACE_INLINE zField3D<T>::zField3D(double _unit_X, double _unit_Y, double _unit_Z, int _n_X, int _n_Y, int _n_Z, zVector _minBB)
		: n_X(_n_X), n_Y(_n_Y), n_Z(_n_Z), unit_X(_unit_X), unit_Y(_unit_Y), unit_Z(_unit_Z), minBB(_minBB)
	{
		maxBB = minBB + zVector(unit_X * n_X, unit_Y * n_Y, unit_Z * n_Z);

		fieldValues.assign(
			static_cast<std::size_t>(n_X) * static_cast<std::size_t>(n_Y) * static_cast<std::size_t>(n_Z),
			T());
	}

	//---- DESTRUCTOR

	template <typename T>
	ZSPACE_INLINE zField3D<T>::~zField3D() {}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
	// explicit instantiation
	template class zField3D<zVector>;

	template class zField3D<float>;

#endif
}
