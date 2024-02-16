/*###############################################################################
#
# Copyright 2022 NVIDIA Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
###############################################################################*/



#if defined(ZSPACE_USD_INTEROP) 

#ifndef ZSPACE_INTEROP_OMNIPRIMUTILS_H
#define ZSPACE_INTEROP_OMNIPRIMUTILS_H

#pragma once
#include <string>
#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <algorithm>

#include "OmniClient.h"

#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/sdf/attributeSpec.h>
#include <pxr/usd/sdf/copyUtils.h>
#include "pxr/usd/usd/common.h"
#include "pxr/usd/usd/editContext.h"
#include "pxr/usd/usd/primCompositionQuery.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usdGeom/gprim.h"
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/shader.h>
#include "pxr/usd/usdUtils/dependencies.h"
#include <pxr/usd/usdUtils/flattenLayerStack.h>
#include "pxr/usd/usdUtils/stitch.h"
#include "pxr/usd/pcp/layerStack.h"

#define FLATTEN_ROOT_LAYER_DURING_MERGE 0

namespace primUtils
{
	namespace FUSDCustomLayerData
	{

		const std::string MutenessCustomKey = "omni_layer:muteness";
		const std::string LockedCustomKey = "omni_layer:locked";
	}

	static const pxr::TfToken customLayerData("customLayerData");

    // These are some utilities borrowed from the Omniverse Connectors to copy, remove, and rename USD prims.
    // They were created because
    //  * multi-layer editing presents challenges when trying to remove a prim (the prim must be disabled, not deleted)
    //  * copying a prim can be complex depending on the prim specifier, reference type, etc
    bool RemovePrim(const pxr::UsdPrim& prim);
    bool CopyPrim(const pxr::UsdPrim& prim, pxr::SdfPath dstPath = pxr::SdfPath());
    bool RenamePrim(const pxr::UsdPrim& prim, const pxr::TfToken& newName);

    void MergeLayers(pxr::SdfLayerRefPtr sourceLayer, pxr::SdfLayerRefPtr targetLayer);
	
    void MergeLiveLayerToRoot(pxr::SdfLayerRefPtr sourceLayer, pxr::SdfLayerRefPtr targetLayer);
    
    void MergeLiveLayerToNewLayer(pxr::SdfLayerRefPtr sourceLayer, pxr::SdfLayerRefPtr rootLayer, const char* targetLayerIdentifier);

    // This makes a relative URL so that references can be relative to their stage:
    //  baseUrl (stage):     omniverse://server/ProjectA/Props/my_mesh.usd
    //  otherUrl (material): omniverse://server/ProjectA/Materials/my_material.usd
    //  returnUrl:           ../Materials/my_material.usd
    std::string MakeRelativeUrl(const char* baseUrl, const char* otherUrl);

    // fileExists returns true if omniClientStat() finds it
    bool fileExists(const char* fileUrl);

    // These functions might be useful for different USD stage/layer operations
    bool RemovePrimSpecInEditLayer(const pxr::UsdPrim& prim);
    pxr::SdfLayerHandle  HasOverriddenInStrongerLayer(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath, bool bOnlyActivePrim = false);
    int  FindSubLayerIndex(const pxr::SdfLayerHandle& rootLayer, const std::string& path);
    bool  SetSubLayerName(const pxr::SdfLayerHandle& rootLayer, int index, const pxr::TfToken& name);
    pxr::SdfLayerRefPtr  CreateSubLayer(const pxr::SdfLayerHandle& rootLayer, int index);
   
    //bool  RemovePrim(const pxr::UsdPrim& prim);
   
    bool  RestorePrim(const pxr::SdfLayerRefPtr& AnonymousLayer, const std::string& LayerIdentifier, const pxr::SdfPath& PrimPath);
   
    //bool  CopyPrim(const pxr::UsdPrim& prim, pxr::SdfPath dstPath = pxr::SdfPath());
   
    //bool  RenamePrim(const pxr::UsdPrim& prim, const pxr::TfToken& newName);
    bool IsAncestralPrim(const pxr::UsdPrim& Prim);
    bool IsAncestorGprim(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath);
    bool CanRemovePrim(const pxr::UsdPrim& prim);
    bool GetIntroducingLayer(const pxr::UsdPrim& prim, pxr::SdfLayerHandle& outLayer, pxr::SdfPath& outPath);
    void ResolvePaths(const std::string& srcLayerIdentifier, const std::string& targetLayerIdentifier, bool storeRelativePath, bool relativeToSrcLayer = false, bool copySublayerLayerOffsets = false);
    bool MergePrimSpec(const std::string& dstLayerIdentifier, const std::string& srcLayerIdentifier, const std::string& primSpecPath, bool isDstStrongerThanSrc, const std::string& targetPrimPath = "");
    std::vector<std::string> GetAllSublayers(const pxr::UsdStageRefPtr& stage, bool includeSessionLayer);
    void ResolvePrimPathReferences(const pxr::SdfLayerRefPtr& layer, const pxr::SdfPath& oldPath, const pxr::SdfPath& newPath);
    bool StitchPrimSpecs(const pxr::UsdPrim& prim, pxr::SdfPath dstPath);
    bool IsLayerExist(const std::string& layerIdentifier);
    void ResolvePathsInternal(const pxr::SdfLayerRefPtr& srcLayer, pxr::SdfLayerRefPtr dstLayer, bool storeRelativePath, bool relativeToSrcLayer = false, bool copyLayerOffsets = false);
    bool MergePrimSpecInternal(pxr::SdfLayerRefPtr dstLayer, const pxr::SdfLayerRefPtr& srcLayer, const pxr::SdfPath& primSpecPath, bool isDstStrongerThanSrc, const pxr::SdfPath& targetPrimPath = pxr::SdfPath::EmptyPath());

    // Returns true if there are any prims defined in the root layer of the stage
    bool RootLayerHasPrimSpecsDefined(pxr::UsdStageRefPtr stage);

    // Remove a property from a prim
   // void removeProperty(const pxr::SdfPath& primPath, const pxr::TfToken& propName);
    void removeProperty(pxr::UsdStageRefPtr stage, const pxr::SdfPath& primPath, const pxr::TfToken& propName);

    // Get the MDL shader prim from a Material prim
    pxr::UsdShadeShader getShaderFromMaterialPrim(const pxr::UsdPrim& prim);

    // Create an input for an MDL shader in a material
    pxr::UsdAttribute createMaterialInput(
        const pxr::UsdPrim& prim,
        const std::string& name,
        const pxr::VtValue& value,
        const pxr::SdfValueTypeName& typeName,
        const pxr::TfToken& colorSpace = pxr::TfToken());

    // These utilities were removed from the AR 2.0 API, so provide them here
    

} // namespace primUtils

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterOp/core/omniUtils/primUtils.cpp>
#endif

#endif

#endif
