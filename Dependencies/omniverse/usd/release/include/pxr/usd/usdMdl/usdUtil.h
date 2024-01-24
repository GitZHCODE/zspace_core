/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/plugin/usdMdl/api.h>
#include <string>

PXR_NAMESPACE_OPEN_SCOPE

/// UsdShadeMaterialToMdl()
///
/// Convert a USD Material to a new MDL material instance
///
/// \param usdMaterial: Input USD material to convert
/// \param mdlMaterialInstanceName: Instance material name, return failure (-6) if this name already exists in the database
///
/// \return 
/// 0: Success
/// -1: Unknown error
/// -2: Can not get module from material
/// -3: Can not load module
/// -4: Can not get material definition from material
/// -5: Can not get material definition from database
/// -6: Can not create material instance
/// -7: Can not get material instance
///
USDMDLAPI_API int UsdShadeMaterialToMdl(const UsdShadeMaterial & usdMaterial, const char * mdlMaterialInstanceName);

PXR_NAMESPACE_CLOSE_SCOPE
