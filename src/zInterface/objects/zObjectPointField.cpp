// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms, 
// data analysis & visualization framework.
//
// Copyright (C) 2019 ZSPACE 
// 
// This Source Code Form is subject to the terms of the MIT License 
// If a copy of the MIT License was not distributed with this file, You can 
// obtain one at https://opensource.org/licenses/MIT.
//
// Author : Vishu Bhooshan <vishu.bhooshan@zaha-hadid.com>, Leo Bieling <leo.bieling@zaha-hadid.com>
//

#include <zspace/zInterface/objects/zObjectPointField.h>
#include <src/zInterface/objects/zFieldObjectStorage.h>

namespace zSpace
{
	ZSPACE_INLINE zObjectPointScalarField::zObjectPointScalarField()
		: zObjectPointCloud(), fieldImpl(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zObjectPointScalarField::zObjectPointScalarField(const zObjectPointScalarField& other)
		: zObjectPointCloud(other), fieldImpl(std::make_unique<Impl>(*other.fieldImpl))
	{
	}

	ZSPACE_INLINE zObjectPointScalarField::zObjectPointScalarField(zObjectPointScalarField&& other) noexcept = default;

	ZSPACE_INLINE zObjectPointScalarField& zObjectPointScalarField::operator=(const zObjectPointScalarField& other)
	{
		if (this != &other)
		{
			zObjectPointCloud::operator=(other);
			*fieldImpl = *other.fieldImpl;
		}
		return *this;
	}

	ZSPACE_INLINE zObjectPointScalarField& zObjectPointScalarField::operator=(zObjectPointScalarField&& other) noexcept = default;

	ZSPACE_INLINE zObjectPointScalarField::~zObjectPointScalarField() = default;

	ZSPACE_INLINE zObjectPointVectorField::zObjectPointVectorField()
		: zObjectPointCloud(), fieldImpl(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zObjectPointVectorField::zObjectPointVectorField(const zObjectPointVectorField& other)
		: zObjectPointCloud(other), fieldImpl(std::make_unique<Impl>(*other.fieldImpl))
	{
	}

	ZSPACE_INLINE zObjectPointVectorField::zObjectPointVectorField(zObjectPointVectorField&& other) noexcept = default;

	ZSPACE_INLINE zObjectPointVectorField& zObjectPointVectorField::operator=(const zObjectPointVectorField& other)
	{
		if (this != &other)
		{
			zObjectPointCloud::operator=(other);
			*fieldImpl = *other.fieldImpl;
		}
		return *this;
	}

	ZSPACE_INLINE zObjectPointVectorField& zObjectPointVectorField::operator=(zObjectPointVectorField&& other) noexcept = default;

	ZSPACE_INLINE zObjectPointVectorField::~zObjectPointVectorField() = default;
}
