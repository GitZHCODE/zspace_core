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


#include<zspace/zInterface/objects/zObjectMeshField.h>

namespace zSpace
{

	//---- CONSTRUCTOR
	
	template<typename T>
	ZSPACE_INLINE zObjectMeshField<T>::zObjectMeshField()
	{
	}

	//---- DESTRUCTOR

	template<typename T>
	ZSPACE_INLINE zObjectMeshField<T>::~zObjectMeshField() {}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
	// explicit instantiation
	template class zObjectMeshField<zVector>;

	template class zObjectMeshField<zScalar>;

#endif
}
