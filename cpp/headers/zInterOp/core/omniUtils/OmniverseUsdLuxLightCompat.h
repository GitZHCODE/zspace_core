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


// In USD 21.02 many UsdLuxLight attributes changed:
// Input attributes for UsdLuxLight and UsdLuxLightFilter schemas are now 
// connectable and have been renamed to include the "inputs:" prefix.
// https://github.com/PixarAnimationStudios/USD/blob/release/CHANGELOG.md#2102---2021-01-18
//
// This utility class helps with this:
//  USD Export: input attributes are created in both their pre and post 21.02 form
//              eg. both "intensity" and "inputs:intensity" are exported
//
//  USD Import:
//    * Light has only "intensity" authored --- return "intensity"
//    * Light has only "inputs:intensity" authored --- return "inputs:intensity"
//    * Light has both "inputs:intensity" and "intensity" authored --- depends on bPreferNewSchema
//

#if defined(ZSPACE_USD_INTEROP) 

#ifndef ZSPACE_INTEROP_OMNIUSDLUXLIGHT_H
#define ZSPACE_INTEROP_OMNIUSDLUXLIGHT_H

#pragma once

// In case we use this outside of Unreal Engine
#if !defined OMNIVERSEUSD_API
    #define OMNIVERSEUSD_API
    #include <pxr/base/tf/token.h>
    #include <pxr/usd/usd/prim.h>
    #include <pxr/usd/usdLux/cylinderLight.h>
    #include <pxr/usd/usdLux/diskLight.h>
    #include <pxr/usd/usdLux/distantLight.h>
    #include <pxr/usd/usdLux/domeLight.h>
    #include <pxr/usd/usdLux/rectLight.h>
    #include <pxr/usd/usdLux/shapingAPI.h>
    #include <pxr/usd/usdLux/sphereLight.h>
    #if PXR_VERSION >= 2111
        #include <pxr/usd/usdLux/lightAPI.h>
    #endif //PXR_VERSION >= 2111
#else
    #include "CoreMinimal.h"
    #include "OmniversePxr.h"
#endif

class OMNIVERSEUSD_API OmniverseUsdLuxLightCompat
{
public:
    // Handle 21.11 changes for the definition of "is a prim a light?"
    //  Added UsdLuxLightAPI, which can be applied to geometry to create a light. 
    //  UsdLuxLight has been deleted. Added the applied API schemas UsdLuxMeshLightAPI 
    //  and UsdLuxVolumeLightAPI, which include UsdLuxLightAPI as a builtin.
    //  https://github.com/PixarAnimationStudios/USD/blob/release/CHANGELOG.md#usd-5
    static bool PrimIsALight(const pxr::UsdPrim& Prim);

    // Universal attribute retrieval for UsdLuxLight attributes updated with the "inputs:" connectible schema change
    static pxr::UsdAttribute GetLightAttr(const pxr::UsdPrim& Light, const pxr::UsdAttribute& DefaultAttr, bool bPreferNewSchema = true);

    // Get a list of both current version and forward/reverse compatible schema attribute names
    static const pxr::TfTokenVector LightGetSchemaAttributeNames();
    static const pxr::TfTokenVector CylinderLightGetSchemaAttributeNames();
    static const pxr::TfTokenVector DiskLightGetSchemaAttributeNames();
    static const pxr::TfTokenVector DistantLightGetSchemaAttributeNames();
    static const pxr::TfTokenVector DomeLightGetSchemaAttributeNames();
    static const pxr::TfTokenVector RectLightGetSchemaAttributeNames();
    static const pxr::TfTokenVector ShapingAPIGetSchemaAttributeNames();
    static const pxr::TfTokenVector SphereLightGetSchemaAttributeNames();
    // Utility method to combine TfTokenVectors
    static const pxr::TfTokenVector CombineWithCompatAttrNames(pxr::TfTokenVector DefaultTokenVec, const pxr::TfTokenVector& CompatTokenVec);

    // UsdLuxDistantLight
    static void CreateAngleAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);

    // UsdLuxDomeLight
    static void CreateTextureFileAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);
    static void CreateTextureFormatAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);

    // UsdLuxSphereLight
    static 	void CreateRadiusAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);

    // UsdLuxShapingAPI
    static void CreateShapingConeAngleAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);
    static void CreateShapingConeSoftnessAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);

    // UsdLuxRectLight
    static void CreateWidthAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);
    static void CreateHeightAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);

    // UsdLuxLightAPI
    static void CreateEnableColorTemperatureAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);
    static void CreateColorTemperatureAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value);
    static void CreateColorAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value, pxr::UsdTimeCode Time = pxr::UsdTimeCode::Default());
    static void CreateIntensityAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value, pxr::UsdTimeCode Time = pxr::UsdTimeCode::Default());


    /////////////////////////////////////////////////////////
    // Internal Helper Methods
    /////////////////////////////////////////////////////////

    // A simple utility for erasing a substring from a string
    // Borrowed from https://thispointer.com/how-to-remove-substrings-from-a-string-in-c/
    static void EraseSubStr(std::string& MainStr, const std::string& ToErase);

    // If version < 21.02, prepend "inputs:" to attribute name 
    // If verseion >= 21.02, remove "inputs:" from attribute name
    static pxr::TfToken CompatName(const pxr::TfToken& OriginalAttrName);
    static pxr::TfToken CompatName(const pxr::UsdAttribute& Attr);

    // These allow us to create, set, and assign an attribute in one line
    static inline pxr::UsdAttribute SetNewAttr(pxr::UsdAttribute NewAttr, const pxr::VtValue& Value, pxr::UsdTimeCode& Time);
    static inline pxr::UsdAttribute SetNewAttr(pxr::UsdAttribute NewAttr, const pxr::VtValue& Value);


    /////////////////////////////////////////////////////////
    // Internal string/token vectors
    /////////////////////////////////////////////////////////
    static constexpr const char* LuxInputsStr = "inputs:";
    static const pxr::TfTokenVector CompatLightAttrs;
    static const pxr::TfTokenVector CompatCylinderLightAttrs;
    static const pxr::TfTokenVector CompatDiskLightAttrs;
    static const pxr::TfTokenVector CompatDistantLightAttrs;
    static const pxr::TfTokenVector CompatDomeLightAttrs;
    static const pxr::TfTokenVector CompatRectLightAttrs;
    static const pxr::TfTokenVector CompatShapingAPIAttrs;
    static const pxr::TfTokenVector CompatSphereLightAttrs;
};

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterOp/core/omniUtils/OmniverseUsdLuxLightCompat.cpp>
#endif


#endif

#endif