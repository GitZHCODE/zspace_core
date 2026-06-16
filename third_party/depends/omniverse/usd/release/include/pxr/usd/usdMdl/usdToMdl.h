/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
#include <mi/mdl_sdk.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/plugin/usdMdl/api.h>
#include <string>

PXR_NAMESPACE_OPEN_SCOPE

/// Convert()
///
/// Convert a USD Material to a new MDL material instance
///
/// \param usdMaterial: Input USD material to convert
/// \param neuray: Interface to MDL SDK
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
USDMDLAPI_API int ConvertUsdMaterialToMdl(const UsdShadeMaterial & usdMaterial, mi::neuraylib::INeuray * neuray, const char * mdlMaterialInstanceName);

PXR_NAMESPACE_CLOSE_SCOPE
