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

#include <zspace/zInterface/objects/zObjectMeshField.h>
#include <src/zInterface/objects/zFieldObjectStorage.h>

namespace zSpace
{
	ZSPACE_INLINE zObjectMeshScalarField::zObjectMeshScalarField()
		: zObjectMesh(), fieldImpl(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zObjectMeshScalarField::zObjectMeshScalarField(const zObjectMeshScalarField& other)
		: zObjectMesh(other), fieldImpl(std::make_unique<Impl>(*other.fieldImpl))
	{
	}

	ZSPACE_INLINE zObjectMeshScalarField::zObjectMeshScalarField(zObjectMeshScalarField&& other) noexcept = default;

	ZSPACE_INLINE zObjectMeshScalarField& zObjectMeshScalarField::operator=(const zObjectMeshScalarField& other)
	{
		if (this != &other)
		{
			zObjectMesh::operator=(other);
			*fieldImpl = *other.fieldImpl;
		}
		return *this;
	}

	ZSPACE_INLINE zObjectMeshScalarField& zObjectMeshScalarField::operator=(zObjectMeshScalarField&& other) noexcept = default;

	ZSPACE_INLINE zObjectMeshScalarField::~zObjectMeshScalarField() = default;

	ZSPACE_INLINE zObjectMeshVectorField::zObjectMeshVectorField()
		: zObjectMesh(), fieldImpl(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zObjectMeshVectorField::zObjectMeshVectorField(const zObjectMeshVectorField& other)
		: zObjectMesh(other), fieldImpl(std::make_unique<Impl>(*other.fieldImpl))
	{
	}

	ZSPACE_INLINE zObjectMeshVectorField::zObjectMeshVectorField(zObjectMeshVectorField&& other) noexcept = default;

	ZSPACE_INLINE zObjectMeshVectorField& zObjectMeshVectorField::operator=(const zObjectMeshVectorField& other)
	{
		if (this != &other)
		{
			zObjectMesh::operator=(other);
			*fieldImpl = *other.fieldImpl;
		}
		return *this;
	}

	ZSPACE_INLINE zObjectMeshVectorField& zObjectMeshVectorField::operator=(zObjectMeshVectorField&& other) noexcept = default;

	ZSPACE_INLINE zObjectMeshVectorField::~zObjectMeshVectorField() = default;
}
