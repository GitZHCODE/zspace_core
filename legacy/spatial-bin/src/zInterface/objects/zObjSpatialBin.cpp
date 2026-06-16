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


#include<zspace/zInterface/objects/zObjSpatialBin.h>

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zObjSpatialBin::zObjSpatialBin()
	{
		displayBounds = false;
	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zObjSpatialBin::~zObjSpatialBin() {}

	//---- SET METHODS

	ZSPACE_INLINE void zObjSpatialBin::setDisplayBounds(bool _displayBounds)
	{
		displayBounds = _displayBounds;
	}

	//---- OVERRIDE METHODS

#if defined (ZSPACE_UNREAL_INTEROP) || defined (ZSPACE_MAYA_INTEROP) /*|| defined (ZSPACE_RHINO_INTEROP)*/
	// Do Nothing
#else

	ZSPACE_INLINE void zObjSpatialBin::draw()
	{
	}

	//---- PROTECTED DISPLAY METHODS

	ZSPACE_INLINE void zObjSpatialBin::drawBins()
	{
	}

	ZSPACE_INLINE void zObjSpatialBin::drawBounds()
	{
	}

#endif //!ZSPACE_UNREAL_INTEROP

}
