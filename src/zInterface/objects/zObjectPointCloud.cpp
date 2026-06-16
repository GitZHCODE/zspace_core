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


#include<zspace/zInterface/objects/zObjectPointCloud.h>
#include <src/zInterface/objects/internal/zObjPointCloudImpl.h>

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjectPointCloud::zObjectPointCloud()
		: impl(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zObjectPointCloud::zObjectPointCloud(const zObjectPointCloud& other)
		: zObject(other),
		  impl(std::make_unique<Impl>(*other.impl))
	{
	}

	ZSPACE_INLINE zObjectPointCloud::zObjectPointCloud(zObjectPointCloud&& other) noexcept = default;

	ZSPACE_INLINE zObjectPointCloud& zObjectPointCloud::operator=(const zObjectPointCloud& other)
	{
		if (this != &other)
		{
			zObject::operator=(other);
			*impl = *other.impl;
		}
		return *this;
	}

	ZSPACE_INLINE zObjectPointCloud& zObjectPointCloud::operator=(zObjectPointCloud&& other) noexcept = default;

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjectPointCloud::~zObjectPointCloud() = default;

}
