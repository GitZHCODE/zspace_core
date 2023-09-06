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

//#include "OmniverseUsdLuxLightCompat.h"
#include<headers/zInterOp/core/omniUtils/OmniverseUsdLuxLightCompat.h>

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatLightAttrs =
{
#if PXR_VERSION >= 2102
    pxr::TfToken("intensity"),
    pxr::TfToken("exposure"),
    pxr::TfToken("diffuse"),
    pxr::TfToken("specular"),
    pxr::TfToken("normalize"),
    pxr::TfToken("color"),
    pxr::TfToken("enableColorTemperature"),
    pxr::TfToken("colorTemperature")
#else
    pxr::TfToken("inputs:intensity"),
    pxr::TfToken("inputs:exposure"),
    pxr::TfToken("inputs:diffuse"),
    pxr::TfToken("inputs:specular"),
    pxr::TfToken("inputs:normalize"),
    pxr::TfToken("inputs:color"),
    pxr::TfToken("inputs:enableColorTemperature"),
    pxr::TfToken("inputs:colorTemperature")
#endif
};

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatCylinderLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("length"),
    pxr::TfToken("radius")
#else
    pxr::TfToken("inputs:length"),
    pxr::TfToken("inputs:radius")
#endif
};
const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatDiskLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("radius")
#else
    pxr::TfToken("inputs:radius")
#endif
};

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatDistantLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("angle"),
    pxr::TfToken("intensity")
#else
    pxr::TfToken("inputs:angle"),
    pxr::TfToken("inputs:intensity")

#endif
};

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatDomeLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("texture:file"),
    pxr::TfToken("texture:format")
#else
    pxr::TfToken("inputs:texture:file"),
    pxr::TfToken("inputs:texture:format")
#endif
};

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatRectLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("width"),
    pxr::TfToken("height"),
    pxr::TfToken("texture:file")
#else
    pxr::TfToken("inputs:width"),
    pxr::TfToken("inputs:height"),
    pxr::TfToken("inputs:texture:file")
#endif
};

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatShapingAPIAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("shaping:focus"),
    pxr::TfToken("shaping:focusTint"),
    pxr::TfToken("shaping:cone:angle"),
    pxr::TfToken("shaping:cone:softness"),
    pxr::TfToken("shaping:ies:file"),
    pxr::TfToken("shaping:ies:angleScale"),
    pxr::TfToken("shaping:ies:normalize")
#else
    pxr::TfToken("inputs:shaping:focus"),
    pxr::TfToken("inputs:shaping:focusTint"),
    pxr::TfToken("inputs:shaping:cone:angle"),
    pxr::TfToken("inputs:shaping:cone:softness"),
    pxr::TfToken("inputs:shaping:ies:file"),
    pxr::TfToken("inputs:shaping:ies:angleScale"),
    pxr::TfToken("inputs:shaping:ies:normalize")
#endif
};

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CompatSphereLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("radius")
#else
    pxr::TfToken("inputs:radius")
#endif
};

// Handle 21.11 changes for the definition of "is a prim a light?"
bool OmniverseUsdLuxLightCompat::PrimIsALight(const pxr::UsdPrim& Prim)
{
#if PXR_VERSION >= 2111
    return Prim.HasAPI<pxr::UsdLuxLightAPI>();
#else
    return Prim.IsA<pxr::UsdLuxLight>();
#endif // PXR_VERSION >= 2111

}

// Universal attribute retrieval for UsdLuxLight attributes updated with the "inputs:" connectible schema change
pxr::UsdAttribute OmniverseUsdLuxLightCompat::GetLightAttr(const pxr::UsdPrim& Light, const pxr::UsdAttribute& DefaultAttr, bool bPreferNewSchema)
{
    // * Light has only "intensity" authored --- return "intensity"
    // * Light has only "inputs:intensity" authored --- return "inputs:intensity"
    // * Light has both "inputs:intensity" and "intensity" authored --- depends on bPreferNewSchema

    pxr::TfToken DefaultAttrName = DefaultAttr.GetName();
    // assume CompatAttr is the one with "inputs:" for now
    pxr::UsdAttribute CompatAttr = Light.GetAttribute(CompatName(DefaultAttrName));
    if (!CompatAttr)
    {
        return DefaultAttr;
    }
    else if (CompatAttr.IsAuthored() && !DefaultAttr.IsAuthored())
    {
        return CompatAttr;
    }

    // In 20.08, the CompatAttr is the new schema
    // In 21.02, the CompatAttr is the old schema
#if PXR_VERSION >= 2102
    return (bPreferNewSchema) ? DefaultAttr : CompatAttr;
#else
    return (bPreferNewSchema) ? CompatAttr : DefaultAttr;
#endif
}


const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(pxr::TfTokenVector DefaultTokenVec, const pxr::TfTokenVector& CompatTokenVec)
{
    DefaultTokenVec.insert(DefaultTokenVec.end(), CompatTokenVec.begin(), CompatTokenVec.end());
    return DefaultTokenVec;
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::LightGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
#if PXR_VERSION >= 2111
        pxr::UsdLuxLightAPI::GetSchemaAttributeNames(false),
#else
        pxr::UsdLuxLight::GetSchemaAttributeNames(false),
#endif //PXR_VERSION >= 2111
        OmniverseUsdLuxLightCompat::CompatLightAttrs);
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::CylinderLightGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxCylinderLight::GetSchemaAttributeNames(false),
        OmniverseUsdLuxLightCompat::CompatCylinderLightAttrs);
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::DiskLightGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxDiskLight::GetSchemaAttributeNames(false),
        OmniverseUsdLuxLightCompat::CompatDiskLightAttrs);
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::DistantLightGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxDistantLight::GetSchemaAttributeNames(false),
        OmniverseUsdLuxLightCompat::CompatDistantLightAttrs);
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::DomeLightGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxDomeLight::GetSchemaAttributeNames(false),
        OmniverseUsdLuxLightCompat::CompatDomeLightAttrs);
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::RectLightGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxRectLight::GetSchemaAttributeNames(false),
        OmniverseUsdLuxLightCompat::CompatRectLightAttrs);
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::ShapingAPIGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxShapingAPI::GetSchemaAttributeNames(false),
        OmniverseUsdLuxLightCompat::CompatShapingAPIAttrs);
}

const pxr::TfTokenVector OmniverseUsdLuxLightCompat::SphereLightGetSchemaAttributeNames()
{
    return OmniverseUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxSphereLight::GetSchemaAttributeNames(false),
        OmniverseUsdLuxLightCompat::CompatSphereLightAttrs);
}


// UsdLuxDistantLight
void OmniverseUsdLuxLightCompat::CreateAngleAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxDistantLight(Light).CreateAngleAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

// UsdLuxDomeLight
void OmniverseUsdLuxLightCompat::CreateTextureFileAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxDomeLight(Light).CreateTextureFileAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void OmniverseUsdLuxLightCompat::CreateTextureFormatAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxDomeLight(Light).CreateTextureFormatAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}


// UsdLuxSphereLight
void OmniverseUsdLuxLightCompat::CreateRadiusAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxSphereLight(Light).CreateRadiusAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

// UsdLuxShapingAPI
void OmniverseUsdLuxLightCompat::CreateShapingConeAngleAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxShapingAPI::Apply(Light).CreateShapingConeAngleAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void OmniverseUsdLuxLightCompat::CreateShapingConeSoftnessAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxShapingAPI::Apply(Light).CreateShapingConeSoftnessAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

// UsdLuxRectLight
void OmniverseUsdLuxLightCompat::CreateWidthAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxRectLight(Light).CreateWidthAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void OmniverseUsdLuxLightCompat::CreateHeightAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxRectLight(Light).CreateHeightAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

// UsdLuxLightAPI
void OmniverseUsdLuxLightCompat::CreateEnableColorTemperatureAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateEnableColorTemperatureAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void OmniverseUsdLuxLightCompat::CreateColorTemperatureAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateColorTemperatureAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void OmniverseUsdLuxLightCompat::CreateColorAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value, pxr::UsdTimeCode Time)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateColorAttr(), Value, Time);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value, Time);
}

void OmniverseUsdLuxLightCompat::CreateIntensityAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value, pxr::UsdTimeCode Time)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateIntensityAttr(), Value, Time);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value, Time);
}


/////////////////////////////////////////////////////////
// Internal Helper Methods
/////////////////////////////////////////////////////////

// A simple utility for erasing a substring from a string
// Borrowed from https://thispointer.com/how-to-remove-substrings-from-a-string-in-c/
void OmniverseUsdLuxLightCompat::EraseSubStr(std::string& MainStr, const std::string& ToErase)
{
    // Search for the substring in string
    size_t Pos = MainStr.find(ToErase);
    if (Pos != std::string::npos)
    {
        // If found then erase it from string
        MainStr.erase(Pos, ToErase.length());
    }
}

// If version < 21.02, prepend "inputs:" to attribute name 
// If verseion >= 21.02, remove "inputs:" from attribute name
pxr::TfToken OmniverseUsdLuxLightCompat::CompatName(const pxr::TfToken& OriginalAttrName)
{
#if PXR_VERSION >= 2102
    // Convert "inputs:intensity" to "intensity"
    std::string AttrName(OriginalAttrName);
    EraseSubStr(AttrName, OmniverseUsdLuxLightCompat::LuxInputsStr);
#else
    // Convert "intensity" to "inputs:intensity"
    std::string AttrName(OmniverseUsdLuxLightCompat::LuxInputsStr);
    AttrName += OriginalAttrName;
#endif
    return pxr::TfToken(AttrName);
    //bool bHasInputsPrefix = pxr::TfStringStartsWith(DefaultAttrName, OmniverseUsdLuxLightCompat::LuxInputsStr);
}

pxr::TfToken OmniverseUsdLuxLightCompat::CompatName(const pxr::UsdAttribute& Attr)
{
    // Convert "intensity" to "inputs:intensity"
    return CompatName(Attr.GetName());
}

// These allow us to create, set, and assign an attribute in one line
inline pxr::UsdAttribute OmniverseUsdLuxLightCompat::SetNewAttr(pxr::UsdAttribute NewAttr, const pxr::VtValue& Value, pxr::UsdTimeCode& Time)
{
    NewAttr.Set(Value, Time);
    return NewAttr;
}
inline pxr::UsdAttribute OmniverseUsdLuxLightCompat::SetNewAttr(pxr::UsdAttribute NewAttr, const  pxr::VtValue& Value)
{
    NewAttr.Set(Value);
    return NewAttr;
}
