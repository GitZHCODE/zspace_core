//
// Copyright 2021 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
////////////////////////////////////////////////////////////////////////
// This file is generated by a script.  Do not edit directly.  Edit the
// schema.template.h file to make changes.

#ifndef PXR_IMAGING_HD_LEGACY_DISPLAY_STYLE_SCHEMA_H
#define PXR_IMAGING_HD_LEGACY_DISPLAY_STYLE_SCHEMA_H

#include "pxr/imaging/hd/api.h"

#include "pxr/imaging/hd/schema.h" 

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------

#define HDLEGACYDISPLAYSTYLE_SCHEMA_TOKENS \
    (displayStyle) \
    (refineLevel) \
    (flatShadingEnabled) \
    (displacementEnabled) \
    (occludedSelectionShowsThrough) \
    (shadingStyle) \
    (reprSelector) \
    (cullStyle) \

TF_DECLARE_PUBLIC_TOKENS(HdLegacyDisplayStyleSchemaTokens, HD_API,
    HDLEGACYDISPLAYSTYLE_SCHEMA_TOKENS);

//-----------------------------------------------------------------------------

class HdLegacyDisplayStyleSchema : public HdSchema
{
public:
    HdLegacyDisplayStyleSchema(HdContainerDataSourceHandle container)
    : HdSchema(container) {}

    //ACCESSORS

    HD_API
    HdIntDataSourceHandle GetRefineLevel();
    HD_API
    HdBoolDataSourceHandle GetFlatShadingEnabled();
    HD_API
    HdBoolDataSourceHandle GetDisplacementEnabled();
    HD_API
    HdBoolDataSourceHandle GetOccludedSelectionShowsThrough();
    HD_API
    HdTokenDataSourceHandle GetShadingStyle();
    HD_API
    HdTokenArrayDataSourceHandle GetReprSelector();
    HD_API
    HdTokenDataSourceHandle GetCullStyle();

    // RETRIEVING AND CONSTRUCTING

    /// Builds a container data source which includes the provided child data
    /// sources. Parameters with nullptr values are excluded. This is a
    /// low-level interface. For cases in which it's desired to define
    /// the container with a sparse set of child fields, the Builder class
    /// is often more convenient and readable.
    HD_API
    static HdContainerDataSourceHandle
    BuildRetained(
        const HdIntDataSourceHandle &refineLevel,
        const HdBoolDataSourceHandle &flatShadingEnabled,
        const HdBoolDataSourceHandle &displacementEnabled,
        const HdBoolDataSourceHandle &occludedSelectionShowsThrough,
        const HdTokenDataSourceHandle &shadingStyle,
        const HdTokenArrayDataSourceHandle &reprSelector,
        const HdTokenDataSourceHandle &cullStyle
    );

    /// \class HdLegacyDisplayStyleSchema::Builder
    /// 
    /// Utility class for setting sparse sets of child data source fields to be
    /// filled as arguments into BuildRetained. Because all setter methods
    /// return a reference to the instance, this can be used in the "builder
    /// pattern" form.
    class Builder
    {
    public:
        HD_API
        Builder &SetRefineLevel(
            const HdIntDataSourceHandle &refineLevel);
        HD_API
        Builder &SetFlatShadingEnabled(
            const HdBoolDataSourceHandle &flatShadingEnabled);
        HD_API
        Builder &SetDisplacementEnabled(
            const HdBoolDataSourceHandle &displacementEnabled);
        HD_API
        Builder &SetOccludedSelectionShowsThrough(
            const HdBoolDataSourceHandle &occludedSelectionShowsThrough);
        HD_API
        Builder &SetShadingStyle(
            const HdTokenDataSourceHandle &shadingStyle);
        HD_API
        Builder &SetReprSelector(
            const HdTokenArrayDataSourceHandle &reprSelector);
        HD_API
        Builder &SetCullStyle(
            const HdTokenDataSourceHandle &cullStyle);

        /// Returns a container data source containing the members set thus far.
        HD_API
        HdContainerDataSourceHandle Build();

    private:
        HdIntDataSourceHandle _refineLevel;
        HdBoolDataSourceHandle _flatShadingEnabled;
        HdBoolDataSourceHandle _displacementEnabled;
        HdBoolDataSourceHandle _occludedSelectionShowsThrough;
        HdTokenDataSourceHandle _shadingStyle;
        HdTokenArrayDataSourceHandle _reprSelector;
        HdTokenDataSourceHandle _cullStyle;
    };

    /// Retrieves a container data source with the schema's default name token
    /// "displayStyle" from the parent container and constructs a
    /// HdLegacyDisplayStyleSchema instance.
    /// Because the requested container data source may not exist, the result
    /// should be checked with IsDefined() or a bool comparison before use.
    HD_API
    static HdLegacyDisplayStyleSchema GetFromParent(
        const HdContainerDataSourceHandle &fromParentContainer);

    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the container representing this schema is found by default.
    HD_API
    static const HdDataSourceLocator &GetDefaultLocator();


    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the reprselector data source can be found.
    /// This is often useful for checking intersection against the
    /// HdDataSourceLocatorSet sent with HdDataSourceObserver::PrimsDirtied.
    HD_API
    static const HdDataSourceLocator &GetReprSelectorLocator();

    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the cullstyle data source can be found.
    /// This is often useful for checking intersection against the
    /// HdDataSourceLocatorSet sent with HdDataSourceObserver::PrimsDirtied.
    HD_API
    static const HdDataSourceLocator &GetCullStyleLocator();

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif