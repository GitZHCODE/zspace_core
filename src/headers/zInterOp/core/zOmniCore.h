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

#if defined(ZSPACE_USD_INTEROP) 

#ifndef ZSPACE_INTEROP_OMNICORE_H
#define ZSPACE_INTEROP_OMNICORE_H

#pragma once
#include "zCore/base/zInline.h"

/// OV
#include "zInterOp/core/omniUtils/xformUtils.h"
#include "zInterOp/core/omniUtils/primUtils.h"
#include "zInterOp/core/omniUtils/OmniverseUsdLuxLightCompat.h"

#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <conio.h>
#endif

#include <math.h>
#include <mutex>
#include <memory>
#include <map>
#include <condition_variable>

#include <OmniClient.h>
#include <OmniUsdResolver.h>
#include "zInterOp/core/omniUtils/OmniChannel.h"
#include "zInterOp/core/omniUtils/LiveSessionInfo.h"
#include "zInterOp/core/omniUtils/LiveSessionConfigFile.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/usd/kind/registry.h>
#include <pxr/usd/usdUtils/pipeline.h>
#include <pxr/usd/usdUtils/sparseValueWriter.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/payloads.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/references.h>
#include <pxr/usd/usdGeom/primvar.h>
#include <pxr/usd/usdShade/input.h>
#include <pxr/usd/usdShade/output.h>
#include <pxr/usd/usdShade/tokens.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usd/usdShade/shader.h>

// Physics includes, note that usdPhysics is now part of USD itself,
// in newer USD versions these includes will be replaced by pxr official usdPhysics schema
#if PXR_VERSION >= 2108
#include <pxr/usd/usdPhysics/scene.h>
#include <pxr/usd/usdPhysics/rigidBodyAPI.h>
#include <pxr/usd/usdPhysics/collisionAPI.h>
#include <pxr/usd/usdPhysics/meshCollisionAPI.h>
#else
#include <usdPhysics/scene.h>
#include <usdPhysics/rigidBodyAPI.h>
#include <usdPhysics/collisionAPI.h>
#include <usdPhysics/meshCollisionAPI.h>
#endif

#if PXR_VERSION >= 2208
#include <pxr/usd/usdGeom/primvarsAPI.h>
#endif

PXR_NAMESPACE_USING_DIRECTIVE


// Private tokens for building up SdfPaths. We recommend
// constructing SdfPaths via tokens, as there is a performance
// cost to constructing them directly via strings (effectively,
// a table lookup per path element). Similarly, any API which
// takes a token as input should use a predefined token
// rather than one created on the fly from a string.
TF_DEFINE_PRIVATE_TOKENS(
	_tokens,
	(DistantLight)
	(DomeLight)
	(Looks)
	(World)
	(Shader)
	(st)

	// These tokens will be reworked or replaced by the official MDL schema for USD.
	// https://developer.nvidia.com/usd/MDLschema
	(Material)
	((_module, "module"))
	(name)
	(out)
	((shaderId, "mdlMaterial"))
	(mdl)

	// Tokens used for USD Preview Surface
	(diffuseColor)
	(normal)
	(file)
	(result)
	(varname)
	(rgb)
	(sourceColorSpace)
	(raw)
	(scale)
	(bias)
	(surface)
	(PrimST)
	(UsdPreviewSurface)
	((UsdShaderId, "UsdPreviewSurface"))
	((PrimStShaderId, "UsdPrimvarReader_float2"))
	(UsdUVTexture)
);

using namespace std;

namespace zSpace
{

	/** \addtogroup zInterOp
	*	\brief classes and function sets for inter operability between maya, rhino and zspace.
	*  @{
	*/

	/** \addtogroup zIO
	*	\brief classes for inter operability between maya, rhino and zspace geometry classes.
	*  @{
	*/

	/*! \class zOmniCore
	*	\brief A Omniverse Core Object Class for loading required Omni modules for interop.
	*	\since version 0.0.4
	*/

	/** @}*/

	/** @}*/



	class ZSPACE_OMNI zOmniCore
	{		

	public:

		//--------------------------
		//---- ATTRIBUTES
		//--------------------------
		 
		// Globals for Omniverse Connection and base Stage
		static UsdStageRefPtr gStage;

		static LiveSessionInfo gLiveSessionInfo;
		static OmniChannel gOmniChannel;

		// Omniverse logging is noisy, only enable it if verbose mode (-v)
		static bool gOmniverseLoggingEnabled;

		// Global for making the logging reasonable
		static mutex gLogMutex;

		static GfVec3d gDefaultRotation;
		static GfVec3i gDefaultRotationOrder;
		static GfVec3d gDefaultScale;

		//--------------------------
		//---- METHODS
		//--------------------------

		void shutdownOmniverse();

		bool startOmniverse();

		bool isValidOmniURL(const std::string& maybeURL);

		void printConnectedUsername(const std::string& stageUrl);

		string createOmniverseModel(const std::string& destinationPath, string stageName, bool doLiveEdit);

		static void OmniClientConnectionStatusCallbackImpl(void* userData, const char* url, OmniClientConnectionStatus status) noexcept;
		
		static void logCallback(const char* threadName, const char* component, OmniClientLogLevel level, const char* message) noexcept;
	
		void failNotify(const char* msg, const char* detail = nullptr);

		void findOrCreateSession(UsdStageRefPtr rootStage, LiveSessionInfo& liveSessionInfo);

		std::string getConnectedUsername(const char* stageUrl);

		bool endAndMergeSession(UsdStageRefPtr rootStage, OmniChannel& channel, LiveSessionInfo& liveSessionInfo);
	
		void checkpointFile(const std::string& stageUrl, const char* comment, bool bForce = true);
	
	};


}

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterOp/core/zOmniCore.cpp>
#endif


#endif

#endif