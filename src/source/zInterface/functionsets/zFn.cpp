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


#include "zInterface/functionsets/zFn.h"

namespace zSpace
{

	//---- CONSTRUCTOR

	ZSPACE_INLINE zFn::zFn()
	{
		fnType = zInvalidFn;

	}

	//---- DESTRUCTOR

	ZSPACE_INLINE zFn::~zFn() {}


	//---- OVERRIDE METHODS

#if defined ZSPACE_USD_INTEROP
	ZSPACE_INLINE bool zFn::usd_openStage(std::string path, UsdStageRefPtr& uStage)
	{
		uStage = UsdStage::Open(path);
		if (!uStage) std::cout << "\n Failure to open stage." << std::endl;
		else cout << "\n opened USD stage of file:  " << path.c_str() << endl;

		return (uStage) ? true : false;
	}

	ZSPACE_INLINE bool zFn::usd_createStage(std::string path, UsdStageRefPtr& uStage)
	{
		uStage = UsdStage::CreateNew(path);
		
		if (!uStage) cout << "\n error creating USD file  " << path.c_str() << endl;
		else
		{
			uStage->SetMetadata(TfToken("defaultPrim"), VtValue("World"));
			uStage->SetMetadata(TfToken("upAxis"), VtValue("Z"));
			uStage->SetMetadata(TfToken("metersPerUnit"), VtValue(1.00));

			UsdGeomXform root = UsdGeomXform::Define(uStage, SdfPath("/World"));
			UsdGeomXform layer = UsdGeomXform::Define(uStage, SdfPath("/World/Geometry"));

			cout << "\n creating USD file: " << path.c_str() << endl;
		}
		
		return (uStage) ? true : false;
	}

#endif
}