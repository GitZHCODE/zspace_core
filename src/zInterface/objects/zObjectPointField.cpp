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


#include<zspace/zInterface/objects/zObjectPointField.h>

namespace zSpace
{

	//---- CONSTRUCTOR

	template<typename T>
	ZSPACE_INLINE zObjectPointField<T>::zObjectPointField()
	{
	}

	//---- DESTRUCTOR

	template<typename T>
	ZSPACE_INLINE zObjectPointField<T>::~zObjectPointField() {}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
	// explicit instantiation
	template class zObjectPointField<zVector>;

	template class zObjectPointField<zScalar>;

#endif
}
