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

#ifndef ZSPACE_INTEROP_OMNIXFORMUTILS_H
#define ZSPACE_INTEROP_OMNIXFORMUTILS_H

#pragma once

#include "pxr/usd/usd/editContext.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usdGeom/xformable.h"
#include "pxr/usd/pcp/layerStack.h"
#include "pxr/base/gf/rotation.h"

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE
inline bool GfIsClose(const PXR_NS::GfQuatd& val1, const PXR_NS::GfQuatd& val2, double tolerance)
{
    bool result1 = PXR_NS::GfIsClose(val1.GetReal(), val2.GetReal(), tolerance) &&
                   GfIsClose(val1.GetImaginary(), val2.GetImaginary(), tolerance);
    bool result2 = GfIsClose(val1.GetReal(), -val2.GetReal(), tolerance) &&
                   GfIsClose(val1.GetImaginary(), -val2.GetImaginary(), tolerance);
    return result1 || result2;
}
PXR_NAMESPACE_CLOSE_SCOPE

namespace
{

/**
 * Gets GfRotation from XformOp that is UsdGeomXformOp::TypeOrient.
 * It is caller's responsibility to pass the xformOp with correct type.
 *
 * @param xformOp The UsdGeomXformOp to get rotation from. It must be of type UsdGeomXformOp::TypeOrient.
 * @param time The timecode to get value from.
 * @return GfRotation of the xformOp. The value is set to identity if failed to fetch from xformOp (mismatched type or undefined value).
 */
template <typename QuatT>
PXR_NS::GfRotation getRotationFromXformOpOrient(const PXR_NS::UsdGeomXformOp& xformOp, const PXR_NS::UsdTimeCode& time);

PXR_NS::SdfLayerRefPtr getLayerIfDefOnSessionOrItsSublayers(PXR_NS::UsdStageRefPtr stage, const PXR_NS::SdfPath& path);

PXR_NS::GfMatrix4d constructTransformMatrixFromSRT(
    const PXR_NS::GfVec3d& translation,
    const PXR_NS::GfVec3d& rotationEuler,
    const PXR_NS::GfVec3i& rotationOrder,
    const PXR_NS::GfVec3d& scale
);

/**
 * Given a target local transform matrix for a prim, determine what value to set just
 * the transformOp when other xformOps are present.
 *
 * @param prim The prim in question
 * @param mtx The desired final transform matrix for the prim including all ops
 * @param foundTransformOp returns true if there is a transform xformOp
 */
PXR_NS::GfMatrix4d findInnerTransform(
    PXR_NS::UsdPrim& prim,
    const PXR_NS::GfMatrix4d& mtx,
    bool& foundTransformOp,
    PXR_NS::UsdTimeCode timecode = PXR_NS::UsdTimeCode::Default(),
    bool skipEqualSetForTimeSample = false
);

/**
 * Checks if a UsdAttribute instance is time sampled.
 *
 * @param attribute The UsdAttribute to be checked.
 * @return True if the attribute is timesampled.
 */
bool isTimeSampled(const PXR_NS::UsdAttribute& attribute);

/**
 * Checks if a UsdAttribute instance has time sample on key timeCode
 *
 * @param attribute The UsdAttribute to be checked.
 * @param timeCode The timeCode to be checked.
 * @return True if the attribute has timesampled on key timeCode.
 */
bool hasTimeSample(const PXR_NS::UsdAttribute& attribute, PXR_NS::UsdTimeCode timeCode);

 /**
 * Defines a helper class to do scoped layer editing. It sets scoped edit target and layer editing permission.
 */
class ScopedLayerEdit
{
public:
    ScopedLayerEdit(PXR_NS::UsdStageWeakPtr stage, const PXR_NS::SdfLayerHandle& layer)
        : m_usdEditCtx(stage, layer), m_layer(layer)
    {
        m_wasLayerEditable = m_layer->PermissionToEdit();
        m_layer->SetPermissionToEdit(true);
    }

    ~ScopedLayerEdit()
    {
        m_layer->SetPermissionToEdit(m_wasLayerEditable);
    }

private:
    PXR_NS::UsdEditContext m_usdEditCtx;
    PXR_NS::SdfLayerHandle m_layer;
    bool m_wasLayerEditable;
};

/**
 * enum to show effective timesamples in layerstacks based on current authoring layer
 */
enum class TimeSamplesOnLayer
{
    eNoTimeSamples,
    eOnCurrentLayer,
    eOnStrongerLayer,
    eOnWeakerLayer
};

/**
 * check if attribute has efficient timesample and
 * these data are on currentlayer/strongerlayer/weakerlayer
 * @tparam Stage  Current Working Stage.
 * @param attribute The attribute to check.
 */
TimeSamplesOnLayer getAttributeEffectiveTimeSampleLayerInfo(const PXR_NS::UsdStage& stage, const PXR_NS::UsdAttribute& attr, PXR_NS::SdfLayerRefPtr* outLayer = nullptr);

/**
 * Copy TimeSample From Waker Layer.
 *
 * @param Stage  Current Working Stage.
 * @param attribute The attribute to check.
 */
void copyTimeSamplesFromWeakerLayer(PXR_NS::UsdStage& stage, const PXR_NS::UsdAttribute& attr);

/**
 * Sets attribute value.
 *
 * @tparam T The data type of the attribute.
 * @param attribute The attribute to set value to.
 * @param val The value to set.
 * @return true if set is successfully.
 */
template <class ValueType>
bool setAttribute(
    const PXR_NS::UsdAttribute& attribute,
    const ValueType& val,
    PXR_NS::UsdTimeCode timeCode = PXR_NS::UsdTimeCode::Default(),
    bool skipEqualSetForTimeSample = false
);

template <class HalfType, class FloatType, class DoubleType, class ValueType>
bool setValueWithPrecision(
    PXR_NS::UsdGeomXformOp& xformOp,
    const ValueType& value,
    PXR_NS::UsdTimeCode timeCode = PXR_NS::UsdTimeCode::Default(),
    bool skipEqualSetForTimeSample = false);

} // namespace

namespace xformUtils
{

/**
 * Gets local transform if applied in scale, rotation, translation order.
 *
 * Depending on the xformOpOrder of the prim, the returned value may not be identical to the final local
 * transform. Only use this function on simple SRT xformOpOrder or matrix.
 *
 * @param prim The prim to get local transform matrix from.
 * @param translation translation to be written to.
 * @param rotation rotation euler angle (in degree) to be written to.
 * @param rotationOrder rotation order to be written to.
 * @param scale scale to be written to.
 * @param time Current timecode.
 * @return true if operation succeed.
 */
    bool getLocalTransformSRT(
        const PXR_NS::UsdPrim& prim,
        PXR_NS::GfVec3d& translation,
        PXR_NS::GfVec3d& rotation,
        PXR_NS::GfVec3i& rotationOrder,
        PXR_NS::GfVec3d& scale,
        PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()
    );

/**
 * Sets local transform matrix of a prim.
 *
 * @param prim The prim to set local transform matrix to.
 * @param mtx The local transform matrix.
 */
bool setLocalTransformSRT(
    PXR_NS::UsdPrim prim,
    const PXR_NS::GfVec3d& translation,
    const PXR_NS::GfVec3d& rotationEuler,
    const PXR_NS::GfVec3i& rotationOrder,
    const PXR_NS::GfVec3d& scale,
    PXR_NS::UsdTimeCode timecode = PXR_NS::UsdTimeCode::Default(),
    bool skipEqualSetForTimeSample = false,
    std::unique_ptr<PXR_NS::SdfChangeBlock>* parentChangeBlock = nullptr
);

} // namespace xformUtils

#if defined(ZSPACE_STATIC_LIBRARY)  || defined(ZSPACE_DYNAMIC_LIBRARY)
// All defined OK so do nothing
#else
#include <source/zInterOp/core/omniUtils/xformUtils.cpp>
#endif

#endif
#endif
