/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
#include <pxr/usd/plugin/usdMdl/api.h>
#include <pxr/usd/plugin/usdMdl/neuray.h>

PXR_NAMESPACE_OPEN_SCOPE

/// SetMdlSdk()
USDMDLAPI_API bool SetMdlSdk(mi::neuraylib::INeuray * neuray);

PXR_NAMESPACE_CLOSE_SCOPE   
