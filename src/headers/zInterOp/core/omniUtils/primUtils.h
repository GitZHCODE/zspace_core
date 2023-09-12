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
    void removeProperty(const pxr::SdfPath& primPath, const pxr::TfToken& propName);

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
    bool IsFileRelative(const std::string& path) {
        return path[0] == '.';
    }
    bool HasScheme(const std::string& path) {
        auto colon = path.find(':');
        return colon != std::string::npos && colon < path.find('/');
    }
    bool IsAbsolute(const std::string& path) {
        return path[0] == '/' || HasScheme(path);
    }
    bool IsSearchPath(const std::string& path) {
        return !IsAbsolute(path) && !IsFileRelative(path);
    }

    bool fileExists(const char* fileUrl)
    {
        bool bFileExists = false;
        omniClientWait(omniClientStat(
            fileUrl,
            &bFileExists,
            [](void* userData, OmniClientResult result, struct OmniClientListEntry const* entry) noexcept
            {
                bool* fileExists = static_cast<bool*>(userData);
                *fileExists = (result == eOmniClientResult_Ok);
            }
        ));
        return bFileExists;
    }

    void MergeLiveLayerToRoot(pxr::SdfLayerRefPtr sourceLayer, pxr::SdfLayerRefPtr targetLayer)
    {
#if FLATTEN_ROOT_LAYER_DURING_MERGE
        // Merge the live deltas to the root layer
        // Make a new temporary stage with two sublayers:
        //  [0] .live layer (source)
        //  [1] the root layer (destination)
        pxr::UsdStageRefPtr flattenStage = pxr::UsdStage::CreateInMemory();
        flattenStage->GetRootLayer()->InsertSubLayerPath(sourceLayer->GetIdentifier());
        flattenStage->GetRootLayer()->InsertSubLayerPath(targetLayer->GetIdentifier());

        // The lamba specified here will be called by SdfCopySpec for every asset encountered.
        // If the asset path doesn't exist we assume it should be left alone
        // If the asset path exists then we attempt to make it a relative path using MakeRelativeUrl()
        //  (new assets in the .live layer will be relative to the .live layer location and that must be fixed)
        pxr::SdfLayerRefPtr flattenedLayer = pxr::UsdUtilsFlattenLayerStack(
            flattenStage,
            [&](const pxr::SdfLayerHandle& layer, const std::string& assetPath)
            {
                // Files like `OmniPBR.mdl` are effectively search paths, unless they actually exist,
                // so don't modify them
                const std::string& assetAbsolutePath = layer->ComputeAbsolutePath(assetPath);
                if (!fileExists(assetAbsolutePath.c_str()))
                {
                    return assetPath;
                }

                // Make this path relative to current layer
                std::string stageAbsPath = targetLayer->GetRealPath();
                return MakeRelativeUrl(stageAbsPath.c_str(), assetAbsolutePath.c_str());
            }
        );

        // Actually copy this flattened layer to the stage's root layer
        pxr::SdfCopySpec(flattenedLayer, pxr::SdfPath::AbsoluteRootPath(), targetLayer, pxr::SdfPath::AbsoluteRootPath());
#else
		for (auto rootPrim : sourceLayer->GetRootPrims())
		{
			MergePrimSpecInternal(targetLayer, sourceLayer, rootPrim->GetPath(), false);
		}	
#endif
    }

    void MergeLiveLayerToNewLayer(pxr::SdfLayerRefPtr sourceLayer, pxr::SdfLayerRefPtr rootLayer, const char* targetLayerIdentifier)
    {
		auto newLayer = pxr::SdfLayer::FindOrOpen(targetLayerIdentifier);
		if (!newLayer)
		{
			newLayer = pxr::SdfLayer::CreateNew(targetLayerIdentifier);
		}
		else
		{
			newLayer->Clear();
		}

		for (auto rootPrim : sourceLayer->GetRootPrims())
		{
			MergePrimSpecInternal(newLayer, sourceLayer, rootPrim->GetPath(), false);
		}	
		newLayer->Save();

		std::string relativePath = MakeRelativeUrl(rootLayer->GetIdentifier().c_str(), targetLayerIdentifier);
		rootLayer->InsertSubLayerPath(relativePath, 0);
	}

    std::string NormalizePath(const std::string& path)
    {
        static auto replaceAll = [](std::string str, const std::string& from, const std::string& to)
        {
            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return str;
        };

        std::string finalPath = path;
        // FIXME: Need a better way to normalize path.
        finalPath = replaceAll(finalPath, "%3C", "<");
        finalPath = replaceAll(finalPath, "%3E", ">");
        finalPath = replaceAll(finalPath, "%20", " ");
        finalPath = replaceAll(finalPath, "%5C", "/");
        std::replace(finalPath.begin(), finalPath.end(), '\\', '/');

        return finalPath;
    }

    std::string ComputeAbsolutePath(const pxr::SdfLayerRefPtr& rootLayer, const std::string& path)
    {
        if (pxr::SdfLayer::IsAnonymousLayerIdentifier(path) || rootLayer->IsAnonymous())
        {
            return path;
        }
        else
        {
            // Compute the path through the resolver
            const std::string& absolutePath = rootLayer->ComputeAbsolutePath(path);
            return NormalizePath(absolutePath);
        }
    }

	std::string MakeRelativeUrl(const char* baseUrl, const char* otherUrl)
	{
		// Borrowed from the omni client python bindings
		size_t urlSize = 100;
		std::string buffer(urlSize, 0);
		char* relativeUrl = nullptr;
		while (relativeUrl == nullptr)
		{
			relativeUrl = omniClientMakeRelativeUrl(baseUrl, otherUrl, &buffer[0], &urlSize);
			buffer.resize(urlSize - 1);
		};
		return buffer;
	}

    std::vector<std::string> LayerTraverse(const pxr::SdfLayerRefPtr& layer, const std::string& identifier)
    {
        std::vector<std::string> layerList;
        layerList.push_back(identifier);

        if (layer)
        {
            for (auto subLayerPath : layer->GetSubLayerPaths())
            {
                auto sublayerIdentifier = layer->ComputeAbsolutePath(subLayerPath);
                auto subLayer = pxr::SdfLayer::FindOrOpen(sublayerIdentifier);
                auto sublayers = LayerTraverse(subLayer, sublayerIdentifier);
                layerList.insert(std::end(layerList), std::begin(sublayers), std::end(sublayers));
            }
        }

        return layerList;
    }


    // Begin static methods from OmniUsdStageCtrl
	bool RemovePrimSpecInEditLayer(const pxr::UsdPrim& prim)
	{
		auto layer = prim.GetStage()->GetEditTarget().GetLayer();
		auto primSpec = layer->GetPrimAtPath(prim.GetPath());
		if (primSpec)
		{
			auto parentSpec = primSpec->GetRealNameParent();
			if (parentSpec)
			{
				parentSpec->RemoveNameChild(primSpec);
				return true;
			}
		}
		return false;
	}


	pxr::SdfLayerHandle HasOverriddenInStrongerLayer(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath, bool bOnlyActivePrim)
	{
		auto LayerStack = Stage->GetLayerStack();

		for (auto Layer : LayerStack)
		{
			if (Layer == Stage->GetEditTarget().GetLayer())
			{
				break;
			}

			auto PrimSpec = Layer->GetPrimAtPath(PrimPath);

			if (!PrimSpec)
			{
				continue;
			}

			if (!bOnlyActivePrim || PrimSpec->HasActive() && PrimSpec->GetActive())
			{
				return Layer;
			}
		}

		return nullptr;
	}

	int FindSubLayerIndex(const pxr::SdfLayerHandle& rootLayer, const std::string& path)
	{
		auto paths = rootLayer->GetSubLayerPaths();

		auto identifier = rootLayer->ComputeAbsolutePath(path);

		for (int i = 0; i < paths.size(); ++i)
		{
			if (rootLayer->ComputeAbsolutePath(paths[i]) == identifier)
				return i;
		}

		return -1;
	}

	bool SetSubLayerName(const pxr::SdfLayerHandle& rootLayer, int index, const pxr::TfToken& name)
	{
		// Remove old name information
		auto subLayerPaths = rootLayer->GetSubLayerPaths();
		if (index < 0 || index >= subLayerPaths.size())
			return false;

		auto nameDict = rootLayer->GetFieldDictValueByKey(pxr::SdfPath::AbsoluteRootPath(), customLayerData, pxr::TfToken("omni_layer:names")).GetWithDefault<pxr::VtDictionary>();

		auto identifier = rootLayer->ComputeAbsolutePath(subLayerPaths[index]);

		for (const auto& entry : nameDict)
		{
			if (rootLayer->ComputeAbsolutePath(entry.first) != identifier)
				continue;

			nameDict.erase(entry.first);
			break;
		}

		// Set new name information
		auto layerFilename = fs::path((std::string)subLayerPaths[index]).filename().string();

		nameDict.SetValueAtPath(layerFilename, pxr::VtValue(name));

		rootLayer->SetFieldDictValueByKey(pxr::SdfPath::AbsoluteRootPath(), customLayerData, pxr::TfToken("omni_layer:names"), pxr::VtValue(nameDict));

		return true;
	}

	pxr::SdfLayerRefPtr CreateSubLayer(const pxr::SdfLayerHandle& rootLayer, int index)
	{
		// Find position
		auto subLayerPaths = rootLayer->GetSubLayerPaths();
		if (index > subLayerPaths.size() || index < -1)
			return nullptr;

		pxr::SdfLayerRefPtr subLayer;
		std::string subLayerPath;

		if (rootLayer->IsAnonymous())
		{
			// Create anonymouse layer
			subLayer = pxr::SdfLayer::CreateAnonymous();
			subLayerPath = subLayer->GetIdentifier();
		}
		else
		{
			// Compose sub layer path
			auto filename = fs::path(rootLayer->GetIdentifier()).filename().stem().stem().string();
			std::stringstream strStream;
			strStream << subLayerPaths.size() + 1;
			filename += "_" + strStream.str();

			std::srand((unsigned int)std::time(NULL));
			auto randomString = [](int length)
			{
				static const char alphanum[] =
					"0123456789"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz";

				std::string result(length, ' ');
				for (int i = 0; i < length; ++i)
				{
					result[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
				}

				return result;
			};

			for (;;)
			{
				// Random string would be the same if two calls happen at the same time. We need to generate again if it happens.
				auto newFilename = filename + "_" + randomString(6) + ".sublayer.usd";

				auto identify = rootLayer->ComputeAbsolutePath("./" + newFilename);
				auto itr = std::find_if(subLayerPaths.begin(), subLayerPaths.end(),
					[&](const std::string& path)
					{
						return rootLayer->ComputeAbsolutePath(path) == identify;
					}
				);

				if (itr == subLayerPaths.end())
				{
					subLayerPath = "./" + newFilename;
					break;
				}
			}

			// Create sub layer
			auto newPath = rootLayer->ComputeAbsolutePath(subLayerPath);

			subLayer = pxr::SdfLayer::FindOrOpen(newPath);
			if (subLayer)
				subLayer->Clear();
			else
				subLayer = pxr::SdfLayer::CreateNew(newPath);
		}

		// Add sub layer
		subLayerPaths.Insert(index, subLayerPath);

		return subLayer;
	}


	// the delete will only happen in the current target, and follows:
	// 1. If the prim spec is a def, it will remove the prim spec.
	// 2. If the prim spec is a over, it will only deactivate this prim.
	// 3. If the prim spec is not existed, it will create over prim and deactivate it.
	// 4. If there is an overridden in a stronger layer, it will report errors.
	bool RemovePrim(const pxr::UsdPrim& prim)
	{
		if (!prim)
		{
			return false;
		}

		pxr::SdfChangeBlock changeBlock;
		auto layer = prim.GetStage()->GetEditTarget().GetLayer();
		auto primPath = prim.GetPath();
		if (CanRemovePrim(prim))
		{
			return RemovePrimSpecInEditLayer(prim);
		}
		else
		{
			if (HasOverriddenInStrongerLayer(prim.GetStage(), primPath, true))
			{
				// TODO: message?
				return false;
			}

			auto primSpec = layer->GetPrimAtPath(prim.GetPath());

			if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
			{
				// remove define at first
				if (RemovePrimSpecInEditLayer(prim))
				{
					// create empty over
					primSpec = pxr::SdfCreatePrimInLayer(layer, primPath);
				}
				else
				{
					return false;
				}
			}
			else
			{
				if (!primSpec)
				{
					// create empty over if there isn't one
					primSpec = pxr::SdfCreatePrimInLayer(layer, primPath);
				}
			}

			// deactive over
			if (primSpec)
			{
				primSpec->SetActive(false);
				return true;
			}
		}

		return false;
	}

	bool RestorePrim(const pxr::SdfLayerRefPtr& AnonymousLayer, const std::string& LayerIdentifier, const pxr::SdfPath& PrimPath)
	{
		pxr::SdfChangeBlock ChangeBlock;

		auto Layer = pxr::SdfLayer::FindOrOpen(LayerIdentifier);
		if (!Layer)
		{
			// Layer has been deleted
			return false;
		}

		auto PreviousPrimSpec = AnonymousLayer->GetPrimAtPath(PrimPath);
		auto CurrentPrimSpec = Layer->GetPrimAtPath(PrimPath);

		// there's an over, remove it
		if (!PreviousPrimSpec && CurrentPrimSpec)
		{
			auto parentSpec = CurrentPrimSpec->GetRealNameParent();
			if (parentSpec)
			{
				parentSpec->RemoveNameChild(CurrentPrimSpec);

				return true;
			}
		}
		// def is removed, recover it
		else if (PreviousPrimSpec)
		{
			if (!CurrentPrimSpec)
			{
				pxr::SdfCopySpec(AnonymousLayer, PrimPath, Layer, PrimPath);
			}
			else
			{
				CurrentPrimSpec->SetActive(true);
			}

			return true;
		}

		return false;
	}

	bool CopyPrim(const pxr::UsdPrim& prim, pxr::SdfPath dstPath)
	{
		auto primPath = prim.GetPath();
		bool bFromReferenceOrPayload = false;
		pxr::SdfLayerHandle introducingLayer;
		pxr::SdfPath introducingPrimPath;
		pxr::SdfPrimSpecHandle primSpecInDefLayer;

		GetIntroducingLayer(prim, introducingLayer, introducingPrimPath);

		for (auto primSpec : prim.GetPrimStack())
		{
			if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
			{
				if (introducingLayer != primSpec->GetLayer() && introducingPrimPath != primPath)
				{
					bFromReferenceOrPayload = true;
					primSpecInDefLayer = primSpec;
					break;
				}
			}
		}

		pxr::SdfChangeBlock changeBlock;
		std::vector<std::string> LayersHasPrimSpec;
		for (auto oldPrimSpec : prim.GetPrimStack())
		{
			if (!oldPrimSpec)
			{
				continue;
			}
			auto layer = oldPrimSpec->GetLayer();
			auto destLayer = prim.GetStage()->GetEditTarget().GetLayer();
			if (oldPrimSpec->HasReferences() || oldPrimSpec->HasPayloads() || oldPrimSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
			{
				pxr::SdfLayerRefPtr srcLayer;
				pxr::SdfPath srcPrimPath;
				if (layer != destLayer)
				{
					auto tempLayer = pxr::SdfLayer::CreateAnonymous();
					pxr::SdfCreatePrimInLayer(tempLayer, primPath);
					pxr::SdfCopySpec(layer, oldPrimSpec->GetPath(), tempLayer, primPath);
					ResolvePaths(layer->GetIdentifier(), tempLayer->GetIdentifier(), false);
					ResolvePaths(destLayer->GetIdentifier(), tempLayer->GetIdentifier(), true, true);

					if (!prim.GetStage()->HasLocalLayer(layer) && bFromReferenceOrPayload)
					{
						auto Prefixes = oldPrimSpec->GetPath().GetPrefixes();
						if (Prefixes.size() > 0)
						{
							ResolvePrimPathReferences(tempLayer, pxr::SdfPath(Prefixes[0]), pxr::SdfPath(introducingPrimPath));
						}
					}

					while (LayersHasPrimSpec.size() > 0)
					{
						auto layerHasPrim = LayersHasPrimSpec[LayersHasPrimSpec.size() - 1];
						if (layerHasPrim != destLayer->GetIdentifier())
						{
							MergePrimSpec(tempLayer->GetIdentifier(), layerHasPrim, primPath.GetString(), false);
						}
						LayersHasPrimSpec.pop_back();
					}

					if (destLayer->GetPrimAtPath(primPath))
					{
						MergePrimSpec(tempLayer->GetIdentifier(), destLayer->GetIdentifier(), primPath.GetString(), false);
					}

					srcLayer = tempLayer;
					srcPrimPath = primPath;
				}
				else
				{
					srcLayer = layer;
					srcPrimPath = oldPrimSpec->GetPath();
				}

				pxr::SdfCreatePrimInLayer(destLayer, dstPath);
				pxr::SdfCopySpec(srcLayer, srcPrimPath, destLayer, dstPath);
				break;
			}
			else
			{
				LayersHasPrimSpec.push_back(layer->GetIdentifier());
			}
		}

		return true;
	}

	bool RenamePrim(const pxr::UsdPrim& prim, const pxr::TfToken& newName)
	{
		if (!prim)
			return false;

		if (prim.IsPseudoRoot())
			return false;

		if (prim.GetPath().GetName() == newName)
			return false;

		auto dstPath = prim.GetPath().GetParentPath().AppendElementToken(newName);

		if (StitchPrimSpecs(prim, dstPath))
		{
			return RemovePrim(prim);
		}

		return false;
	}

	bool IsAncestralPrim(const pxr::UsdPrim& Prim)
	{
		if (Prim)
		{
			auto PrimIndex = Prim.GetPrimIndex();
			// walk the nodes, strong-to-weak
			for (auto Node : PrimIndex.GetNodeRange())
			{
				if (Node.IsDueToAncestor())
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IsAncestorGprim(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath)
	{
		auto ParentPath = PrimPath;
		while (ParentPath != pxr::SdfPath::AbsoluteRootPath())
		{
			auto Prim = Stage->GetPrimAtPath(ParentPath);
			if (Prim && Prim.IsA<pxr::UsdGeomGprim>())
			{
				return true;
			}
			ParentPath = ParentPath.GetParentPath();
		}

		return false;
	}


	bool CanRemovePrim(const pxr::UsdPrim& prim)
	{
		if (IsAncestralPrim(prim))
		{
			return false;
		}

		auto editLayer = prim.GetStage()->GetEditTarget().GetLayer();

		bool bHasDelta = false;
		for (auto layer : prim.GetStage()->GetLayerStack())
		{
			auto primSpec = layer->GetPrimAtPath(prim.GetPath());
			if (!primSpec)
			{
				continue;
			}

			if (layer != editLayer && !bHasDelta && !layer->IsAnonymous())
			{
				bHasDelta = true;
			}
		}

		return !bHasDelta;
	}

	bool GetIntroducingLayer(const pxr::UsdPrim& prim, pxr::SdfLayerHandle& outLayer, pxr::SdfPath& outPath)
	{
		outLayer = nullptr;
		outPath = pxr::SdfPath();
		auto primStack = prim.GetPrimStack();

		for (auto primSpec : primStack)
		{
			if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
			{
				outLayer = primSpec->GetLayer();
				outPath = primSpec->GetPath();
				break;
			}
		}

		if (outLayer.IsInvalid())
		{
			return false;
		}

		auto query = pxr::UsdPrimCompositionQuery(prim);
		auto filter = pxr::UsdPrimCompositionQuery::Filter();
		filter.arcTypeFilter = pxr::UsdPrimCompositionQuery::ArcTypeFilter::ReferenceOrPayload;
		filter.arcIntroducedFilter = pxr::UsdPrimCompositionQuery::ArcIntroducedFilter::IntroducedInRootLayerStack;
		query.SetFilter(filter);
		auto arcs = query.GetCompositionArcs();
		for (auto arc : arcs)
		{
			auto layer = arc.GetIntroducingLayer();
			// Reference
			if (arc.GetArcType() == pxr::PcpArcTypeReference)
			{
				pxr::SdfReferenceEditorProxy proxy;
				pxr::SdfReference ref;

				if (arc.GetIntroducingListEditor(&proxy, &ref))
				{
					auto assetPath = layer->ComputeAbsolutePath(ref.GetAssetPath());
					if (NormalizePath(outLayer->GetIdentifier()) == NormalizePath(assetPath))
					{
						outLayer = layer;
						outPath = arc.GetIntroducingPrimPath();
						break;
					}
				}
			}
			else if (arc.GetArcType() == pxr::PcpArcTypePayload)
			{
				pxr::SdfPayloadEditorProxy proxy;
				pxr::SdfPayload payload;

				if (arc.GetIntroducingListEditor(&proxy, &payload))
				{
					auto assetPath = layer->ComputeAbsolutePath(payload.GetAssetPath());
					if (NormalizePath(outLayer->GetIdentifier()) == NormalizePath(assetPath))
					{
						outLayer = layer;
						outPath = arc.GetIntroducingPrimPath();
						break;
					}
				}
			}
		}

		return true;
	}

	void ResolvePaths(const std::string& srcLayerIdentifier,
		const std::string& targetLayerIdentifier,
		bool storeRelativePath,
		bool relativeToSrcLayer,
		bool copySublayerLayerOffsets)
	{
		auto srcLayer = pxr::SdfLayer::Find(srcLayerIdentifier);
		auto dstLayer = pxr::SdfLayer::Find(targetLayerIdentifier);
		if (!srcLayer || !dstLayer)
		{
			return;
		}

		ResolvePathsInternal(srcLayer, dstLayer, storeRelativePath, relativeToSrcLayer, copySublayerLayerOffsets);
	}

	bool MergePrimSpec(const std::string& dstLayerIdentifier,
		const std::string& srcLayerIdentifier,
		const std::string& primSpecPath,
		bool isDstStrongerThanSrc,
		const std::string& targetPrimPath)
	{
		auto dstLayer = pxr::SdfLayer::Find(dstLayerIdentifier);
		auto srcLayer = pxr::SdfLayer::Find(srcLayerIdentifier);
		if (!dstLayer || !srcLayer)
		{
			return false;
		}

		pxr::SdfPath sdfPrimPath;
		pxr::SdfPath sdfTargetPrimPath;
		if (primSpecPath.empty())
		{
			sdfPrimPath = pxr::SdfPath::EmptyPath();
		}
		else
		{
			sdfPrimPath = pxr::SdfPath(primSpecPath);
		}

		if (targetPrimPath.empty())
		{
			sdfTargetPrimPath = pxr::SdfPath::EmptyPath();
		}
		else
		{
			sdfTargetPrimPath = pxr::SdfPath(targetPrimPath);
		}

		return MergePrimSpecInternal(
			dstLayer, srcLayer, sdfPrimPath, isDstStrongerThanSrc, sdfTargetPrimPath);
	}

	std::vector<std::string> GetAllSublayers(const pxr::UsdStageRefPtr& stage, bool includeSessionLayer)
	{
		std::vector<std::string> allLayers;

		if (includeSessionLayer)
		{
			allLayers = LayerTraverse(stage->GetSessionLayer(), stage->GetSessionLayer()->GetIdentifier());
		}

		std::vector<std::string> layersInRoot = LayerTraverse(stage->GetRootLayer(), stage->GetRootLayer()->GetIdentifier());
		allLayers.insert(std::end(allLayers), std::begin(layersInRoot), std::end(layersInRoot));

		return allLayers;
	}

	void ResolvePrimPathReferences(const pxr::SdfLayerRefPtr& layer, const pxr::SdfPath& oldPath, const pxr::SdfPath& newPath)
	{
		static auto updatePrimPathRef = [](const pxr::SdfPrimSpecHandle& primSpec, const pxr::SdfPath& oldPath, const pxr::SdfPath& newPath)
		{
			auto modifyItemEditsCallback = [&oldPath, &newPath](const pxr::SdfPath& path)
			{
				return path.ReplacePrefix(oldPath, newPath);
			};

			auto modifyItemReferencesCallback = [&oldPath, &newPath](const pxr::SdfReference& reference)
			{
				pxr::SdfPath primPath;
				if (reference.GetAssetPath().empty())
				{
					primPath = reference.GetPrimPath().ReplacePrefix(oldPath, newPath);
				}
				else
				{
					primPath = reference.GetPrimPath();
				}

				return pxr::SdfReference(
					reference.GetAssetPath(),
					primPath,
					reference.GetLayerOffset(),
					reference.GetCustomData()
				);
			};

			// Update relationships
			for (const auto& relationship : primSpec->GetRelationships())
			{
				relationship->GetTargetPathList().ModifyItemEdits(modifyItemEditsCallback);
			}

			// Update connections
			for (const auto& attribute : primSpec->GetAttributes())
			{
				attribute->GetConnectionPathList().ModifyItemEdits(modifyItemEditsCallback);
			}

			primSpec->GetReferenceList().ModifyItemEdits(modifyItemReferencesCallback);
		};

		auto onPrimSpecPath = [&layer, &oldPath, &newPath](const pxr::SdfPath& primPath)
		{
			if (primPath.IsPropertyPath() || primPath == pxr::SdfPath::AbsoluteRootPath())
			{
				return;
			}

			auto primSpec = layer->GetPrimAtPath(primPath);
			if (primSpec)
			{
				updatePrimPathRef(primSpec, oldPath, newPath);
			}
		};

		layer->Traverse(pxr::SdfPath::AbsoluteRootPath(), onPrimSpecPath);
	}

	bool StitchPrimSpecs(const pxr::UsdPrim& prim, pxr::SdfPath dstPath)
	{
		auto primPath = prim.GetPath();
		auto editLayer = prim.GetStage()->GetEditTarget().GetLayer();

		bool bFromReferenceOrPayload = false;
		pxr::SdfLayerHandle introducingLayer;
		pxr::SdfPath introducingPrimPath;
		pxr::SdfPrimSpecHandle primSpecInDefLayer;

		GetIntroducingLayer(prim, introducingLayer, introducingPrimPath);
		for (auto primSpec : prim.GetPrimStack())
		{
			if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
			{
				if (introducingLayer != primSpec->GetLayer() && introducingPrimPath != primPath)
				{
					bFromReferenceOrPayload = true;
					primSpecInDefLayer = primSpec;
					break;
				}
			}
		}

		auto Sublayers = GetAllSublayers(prim.GetStage(), true);

		pxr::SdfChangeBlock changeBlock;

		auto tempLayer = pxr::SdfLayer::CreateAnonymous();
		pxr::SdfCreatePrimInLayer(tempLayer, primPath);
		for (auto subLayerIdentifier : Sublayers)
		{
			pxr::SdfLayerRefPtr subLayer;
			if (bFromReferenceOrPayload && introducingLayer && introducingLayer->GetIdentifier() == subLayerIdentifier)
			{
				subLayer = pxr::SdfLayer::CreateAnonymous();
				pxr::SdfCreatePrimInLayer(subLayer, primPath);
				pxr::SdfCopySpec(primSpecInDefLayer->GetLayer(), primSpecInDefLayer->GetPath(), subLayer, primPath);
			}
			else
			{
				subLayer = pxr::SdfLayer::FindOrOpen(subLayerIdentifier);
			}

			if (subLayer)
			{
				auto srcPrimSpec = subLayer->GetPrimAtPath(primPath);
				if (srcPrimSpec)
				{
					MergePrimSpec(tempLayer->GetIdentifier(), subLayer->GetIdentifier(), primPath.GetString(), true);
				}
			}
		}

		pxr::SdfCreatePrimInLayer(editLayer, dstPath);
		ResolvePaths(editLayer->GetIdentifier(), tempLayer->GetIdentifier(), true, true);
		pxr::SdfCopySpec(tempLayer, primPath, editLayer, dstPath);

		return true;
	}

	bool IsLayerExist(const std::string& layerIdentifier)
	{
		auto layer = pxr::SdfLayer::FindOrOpen(layerIdentifier);
		return layer != nullptr;
	}

	void ResolvePathsInternal(const pxr::SdfLayerRefPtr& srcLayer,
		pxr::SdfLayerRefPtr dstLayer,
		bool storeRelativePath,
		bool relativeToSrcLayer,
		bool copyLayerOffsets)
	{
		using PathConvertFn = std::function<std::string(const std::string& path)>;
		PathConvertFn makePathAbsolute = [&srcLayer, &dstLayer](const std::string& path)
		{
			if (path.empty())
			{
				return path;
			}

			std::string externRefPathFull;
			if (!srcLayer->IsAnonymous())
			{
				externRefPathFull = ComputeAbsolutePath(srcLayer, path);
			}
			else
			{
				externRefPathFull = ComputeAbsolutePath(dstLayer, path);
			}

            // IsSearchPath was removed from AR 2.0
            // See https://graphics.pixar.com/usd/release/wp_ar2.html#add-identifier-concept
			if (IsSearchPath(path) && !fileExists(externRefPathFull.c_str()))
			{
				return path;
			}

			if (externRefPathFull.empty())
			{
				// If it failed to compute the absolute path, just returning the original one.
				return path;
			}
			else
			{
				return externRefPathFull;
			}
		};

		PathConvertFn makePathRelative = [&srcLayer, &dstLayer, relativeToSrcLayer](const std::string& path)
		{
			if (path.empty())
			{
				return path;
			}

			std::string relativePath = ComputeAbsolutePath(srcLayer, path);
			// FIXME: Resolver will firstly find MDL in the same dir as USD
			// for material reference, then Core Library. Currently, this is
			// used to check the existence of the path and see if it's necessary
			// to resolve path of mdl references.

            // IsSearchPath was removed from AR 2.0
            // See https://graphics.pixar.com/usd/release/wp_ar2.html#add-identifier-concept
			if (IsSearchPath(path) && !fileExists(relativePath.c_str()))
			{
				return path;
			}

			if (relativePath.empty() || pxr::SdfLayer::IsAnonymousLayerIdentifier(relativePath))
			{
				return path;
			}
			else
			{
				// Remove old omni: prefix
				if (relativePath.size() >= 5 && relativePath.substr(0, 5) == "omni:")
				{
					relativePath = relativePath.substr(5);
				}
				if (relativeToSrcLayer)
				{
					relativePath = MakeRelativeUrl(srcLayer->GetIdentifier().c_str(), relativePath.c_str());
				}
				else
				{
					relativePath = MakeRelativeUrl(dstLayer->GetIdentifier().c_str(), relativePath.c_str());
				}
				relativePath = NormalizePath(relativePath);

				// If relative path cannot be computed, it returns absolute path to avoid
				// reference issue. For example, if src and dst are not in the same domain.
				return relativePath;
			}
		};

		// Save offsets and scales.
		const auto& layerOffsets = srcLayer->GetSubLayerOffsets();

		PathConvertFn convertPath = storeRelativePath ? makePathRelative : makePathAbsolute;
		pxr::UsdUtilsModifyAssetPaths(
			dstLayer, [&convertPath](const std::string& assetPath) { return convertPath(assetPath); });

		// Copy sublayer offsets
		if (copyLayerOffsets)
		{
			for (size_t i = 0; i < layerOffsets.size(); i++)
			{
				const auto& layerOffset = layerOffsets[i];
				dstLayer->SetSubLayerOffset(layerOffset, static_cast<int>(i));
			}
		}

		// Resolve paths saved in customdata.
		pxr::VtDictionary valueMap;
		pxr::VtDictionary rootLayerCustomData = dstLayer->GetCustomLayerData();
		const auto& customDataValue = rootLayerCustomData.GetValueAtPath(FUSDCustomLayerData::MutenessCustomKey);
		if (customDataValue && !customDataValue->IsEmpty())
		{
			valueMap = customDataValue->Get<pxr::VtDictionary>();
		}

		pxr::VtDictionary newValueMap;
		for (const auto& valuePair : valueMap)
		{
			const std::string& absolutePath = srcLayer->ComputeAbsolutePath(valuePair.first);
			const std::string& relativePath = convertPath(absolutePath);
			newValueMap[relativePath] = valuePair.second;
		}

		rootLayerCustomData.SetValueAtPath(FUSDCustomLayerData::MutenessCustomKey, pxr::VtValue(newValueMap));
		dstLayer->SetCustomLayerData(rootLayerCustomData);
	}

	bool MergePrimSpecInternal(pxr::SdfLayerRefPtr dstLayer,
		const pxr::SdfLayerRefPtr& srcLayer,
		const pxr::SdfPath& primSpecPath,
		bool isDstStrongerThanSrc,
		const pxr::SdfPath& targetPrimPath)
	{
		if (dstLayer == srcLayer)
		{
			// If target path is not the same as original path, it means a duplicate.
			if (!targetPrimPath.IsEmpty() && primSpecPath != targetPrimPath)
			{
				pxr::SdfCopySpec(srcLayer, primSpecPath, dstLayer, targetPrimPath);
				return true;
			}

			return false;
		}

		if (!srcLayer->HasSpec(primSpecPath) && !dstLayer->HasSpec(primSpecPath))
		{
			return false;
		}

		auto originalStrongLayer = isDstStrongerThanSrc ? dstLayer : srcLayer;
		auto originalWeakLayer = isDstStrongerThanSrc ? srcLayer : dstLayer;
		auto targetLayer = dstLayer;

		// srcLayer is weak and dst is strong
		auto shouldCopyValueFn = [targetLayer](const pxr::TfToken& field, const pxr::SdfPath& path,
			const pxr::SdfLayerHandle& strongLayer, bool fieldInStrong,
			const pxr::SdfLayerHandle& weakLayer, bool fieldInWeak,
			pxr::VtValue* valueToCopy)
		{
			pxr::UsdUtilsStitchValueStatus status = pxr::UsdUtilsStitchValueStatus::UseDefaultValue;
			bool handleSublayers = false;
			if (field == pxr::SdfFieldKeys->SubLayers)
			{
				handleSublayers = true;
				status = pxr::UsdUtilsStitchValueStatus::UseSuppliedValue;
			}
			else if (fieldInWeak && fieldInStrong && field == pxr::SdfFieldKeys->Specifier)
			{
				const auto& sObj = weakLayer->GetObjectAtPath(path);
				const auto& dObj = strongLayer->GetObjectAtPath(path);
				const auto& sSpec = sObj->GetField(field).Get<pxr::SdfSpecifier>();
				const auto& dSpec = dObj->GetField(field).Get<pxr::SdfSpecifier>();

				// if either is not an over, we want the new specifier to be whatever that is.
				if (sSpec != pxr::SdfSpecifier::SdfSpecifierOver && dSpec != pxr::SdfSpecifier::SdfSpecifierOver)
				{

				}
				if (sSpec != pxr::SdfSpecifier::SdfSpecifierOver)
				{
					*valueToCopy = pxr::VtValue(sSpec);
					status = pxr::UsdUtilsStitchValueStatus::UseSuppliedValue;
				}
				else if (dSpec != pxr::SdfSpecifier::SdfSpecifierOver)
				{
					*valueToCopy = pxr::VtValue(dSpec);
					status = pxr::UsdUtilsStitchValueStatus::UseSuppliedValue;
				}
			}

			if (handleSublayers)
			{
				// Merge sublayers list between src and dst.
				pxr::SdfSubLayerProxy weakSublayerProxy = weakLayer->GetSubLayerPaths();
				pxr::SdfSubLayerProxy strongSublayerProxy = strongLayer->GetSubLayerPaths();
				std::vector<std::string> mergedSublayerList;
				std::unordered_set<std::string> uniqueSublayers;

				for (size_t i = 0; i < strongSublayerProxy.size(); i++)
				{
					std::string sublayer = strongSublayerProxy[i];
					if (uniqueSublayers.find(sublayer) == uniqueSublayers.end())
					{
						mergedSublayerList.push_back(sublayer);
						uniqueSublayers.insert(sublayer);
					}
				}

				for (size_t i = 0; i < weakSublayerProxy.size(); i++)
				{
					std::string sublayer = weakSublayerProxy[i];
					if (uniqueSublayers.find(sublayer) == uniqueSublayers.end())
					{
						mergedSublayerList.push_back(sublayer);
						uniqueSublayers.insert(sublayer);
					}
				}

				*valueToCopy = pxr::VtValue::Take(mergedSublayerList);
			}

			return status;
		};

		if (!srcLayer->HasSpec(primSpecPath))
		{
			return true;
		}

		auto tempStrongLayer = pxr::SdfLayer::CreateAnonymous();
		auto tempWeakLayer = pxr::SdfLayer::CreateAnonymous();
		auto tempPath = primSpecPath.IsAbsoluteRootPath() ? pxr::SdfPath::AbsoluteRootPath() : pxr::SdfPath::AbsoluteRootPath().AppendElementToken(primSpecPath.GetNameToken());
		pxr::SdfCreatePrimInLayer(tempStrongLayer, tempPath);
		pxr::SdfCreatePrimInLayer(tempWeakLayer, tempPath);
		if (originalStrongLayer->GetPrimAtPath(primSpecPath))
		{
			pxr::SdfCopySpec(originalStrongLayer, primSpecPath, tempStrongLayer, tempPath);
			ResolvePathsInternal(originalStrongLayer, tempStrongLayer, false);
		}
		if (originalWeakLayer->GetPrimAtPath(primSpecPath))
		{
			pxr::SdfCopySpec(originalWeakLayer, primSpecPath, tempWeakLayer, tempPath);
			ResolvePathsInternal(originalWeakLayer, tempWeakLayer, false);
		}
		pxr::UsdUtilsStitchLayers(tempStrongLayer, tempWeakLayer, shouldCopyValueFn);
		ResolvePathsInternal(targetLayer, tempStrongLayer, true, true);

		pxr::SdfCreatePrimInLayer(targetLayer, primSpecPath);
		pxr::SdfPath newPrimPath;
		if (targetPrimPath != pxr::SdfPath::EmptyPath())
		{
			newPrimPath = targetPrimPath;
		}
		else
		{
			newPrimPath = primSpecPath;
		}
		pxr::SdfCopySpec(tempStrongLayer, tempPath, targetLayer, newPrimPath);

		return true;
	}

    bool RootLayerHasPrimSpecsDefined(pxr::UsdStageRefPtr stage)
    {
        bool bHasAnySpecs = false;
        pxr::SdfLayerHandle rootLayer = stage->GetRootLayer();
        rootLayer->Traverse(pxr::SdfPath::AbsoluteRootPath(),
            [&](const pxr::SdfPath& InPath)
            {
                if (!bHasAnySpecs && !InPath.IsAbsoluteRootPath())
                {
                    auto prim = stage->GetPrimAtPath(InPath);
                    if (prim)
                    {
                        for (auto primSpec : prim.GetPrimStack())
                        {
                            if (primSpec && primSpec->GetLayer() == rootLayer)
                            {
                                bHasAnySpecs = true;
                                break;
                            }
                        }
                    }
                }
            });

        return bHasAnySpecs;
    }

    // Remove a property from a prim
    void removeProperty(pxr::UsdStageRefPtr stage, const pxr::SdfPath& primPath, const pxr::TfToken& propName)
    {
        pxr::SdfChangeBlock();
        for (const pxr::SdfLayerHandle& layer : stage->GetLayerStack())
        {
            pxr::SdfPrimSpecHandle primSpec = layer->GetPrimAtPath(primPath);
            if (primSpec)
            {
                pxr::SdfPropertySpecHandle propSpec = layer->GetPropertyAtPath(primPath.AppendProperty(propName));
                if (propSpec)
                {
                    primSpec->RemoveProperty(propSpec);
                }
            }
        }
    }

    // Get the MDL shader prim from a Material prim
    pxr::UsdShadeShader getShaderFromMaterialPrim(const pxr::UsdPrim& prim)
    {
        auto matPrim = pxr::UsdShadeMaterial(prim);
        if (matPrim)
        {
            pxr::UsdShadeShader shader = matPrim.ComputeSurfaceSource({ pxr::TfToken("mdl") });
            return shader;
        }
        return pxr::UsdShadeShader();
    }

    // Create an input for an MDL shader in a material
    pxr::UsdAttribute createMaterialInput(
        const pxr::UsdPrim& prim,
        const std::string& name,
        const pxr::VtValue& value,
        const pxr::SdfValueTypeName& typeName,
        const pxr::TfToken& colorSpace)
    {
        pxr::UsdShadeShader shader = getShaderFromMaterialPrim(prim);
        if (shader)
        {
            pxr::UsdShadeInput existingInput = shader.GetInput(pxr::TfToken(name));
            if (existingInput && existingInput.GetTypeName() != typeName)
            {
                removeProperty(prim.GetStage(), shader.GetPrim().GetPath(), existingInput.GetFullName());
            }

            pxr::UsdShadeInput surfaceInput = shader.CreateInput(pxr::TfToken(name), typeName);
            if (surfaceInput)
            {
                surfaceInput.Set(value);
                const pxr::UsdAttribute& attr = surfaceInput.GetAttr();
                if (!colorSpace.IsEmpty())
                {
                    attr.SetColorSpace(colorSpace);
                }
                return attr;
            }
        }
        return pxr::UsdAttribute();
    }

} // namespace primUtils
