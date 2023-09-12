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
PXR_NS::GfRotation getRotationFromXformOpOrient(const PXR_NS::UsdGeomXformOp& xformOp, const PXR_NS::UsdTimeCode& time)
{
    QuatT quat;
    if (xformOp.Get<QuatT>(&quat, time))
    {
        return PXR_NS::GfRotation(quat);
    }
    return PXR_NS::GfRotation({ 1, 0, 0 }, 0);
}

PXR_NS::SdfLayerRefPtr getLayerIfDefOnSessionOrItsSublayers(PXR_NS::UsdStageRefPtr stage, const PXR_NS::SdfPath& path)
{
    auto sessionLayer = stage->GetSessionLayer();

    auto hasDefSpec = [&path](PXR_NS::SdfLayerRefPtr layer) {
        auto primSpec = layer->GetPrimAtPath(path);
        return primSpec && SdfIsDefiningSpecifier(primSpec->GetSpecifier());
    };

    if (hasDefSpec(sessionLayer))
    {
        return sessionLayer;
    }

    auto sublayerPaths = sessionLayer->GetSubLayerPaths();
    for (auto const& path : sublayerPaths)
    {
        auto layer = PXR_NS::SdfLayer::Find(path);
        if (layer)
        {
            if (hasDefSpec(layer))
            {
                return layer;
            }
        }
    }

    return nullptr;
}

PXR_NS::GfMatrix4d constructTransformMatrixFromSRT(
    const PXR_NS::GfVec3d& translation,
    const PXR_NS::GfVec3d& rotationEuler,
    const PXR_NS::GfVec3i& rotationOrder,
    const PXR_NS::GfVec3d& scale
)
{
    using namespace PXR_NS;

    GfMatrix4d transMtx, rotMtx, scaleMtx;

    transMtx.SetTranslate(translation);

    static const GfVec3d kAxes[] = { GfVec3d::XAxis(), GfVec3d::YAxis(), GfVec3d::ZAxis() };
    GfRotation rotation = GfRotation(kAxes[rotationOrder[0]], rotationEuler[rotationOrder[0]]) *
        GfRotation(kAxes[rotationOrder[1]], rotationEuler[rotationOrder[1]]) *
        GfRotation(kAxes[rotationOrder[2]], rotationEuler[rotationOrder[2]]);

    rotMtx.SetRotate(rotation);

    scaleMtx.SetScale(scale);

    return scaleMtx * rotMtx * transMtx;
}

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
)
{
    using namespace PXR_NS;
    
    UsdGeomXformable xform(prim);

    bool resetXFormStack;
    auto xformOps = xform.GetOrderedXformOps(&resetXFormStack);
    foundTransformOp = false;
    bool foundOtherOps = false;
    GfMatrix4d preTransform = GfMatrix4d(1.);
    GfMatrix4d postTransform = GfMatrix4d(1.);

    for (auto xformOp : xformOps)
    {
        if (!foundTransformOp && xformOp.GetOpType() == UsdGeomXformOp::TypeTransform)
        {
            foundTransformOp = true;
        }
        else
        {
            bool isInverseOp = false;
            UsdGeomXformOp op(xformOp.GetAttr(), isInverseOp);
            if (op)
            {
                static const TfToken kPivotSuffix("pivot");
                if (op.HasSuffix(kPivotSuffix))
                {
                    continue;
                }

                // possibly check for identity and skip multiplication
                auto opTransform = op.GetOpTransform(timecode);
                if (foundTransformOp)
                {
                    preTransform = opTransform * preTransform;
                }
                else
                {
                    postTransform = opTransform * postTransform;
                }
                foundOtherOps = true;
            }
        }
    }

    if (foundTransformOp && foundOtherOps)
    {
        return preTransform.GetInverse() * mtx * postTransform.GetInverse();
    }

    return mtx;
}

/**
 * Checks if a UsdAttribute instance is time sampled.
 *
 * @param attribute The UsdAttribute to be checked.
 * @return True if the attribute is timesampled.
 */
bool isTimeSampled(const PXR_NS::UsdAttribute& attribute)
{
    return attribute.GetNumTimeSamples() > 0;
}

/**
 * Checks if a UsdAttribute instance has time sample on key timeCode
 *
 * @param attribute The UsdAttribute to be checked.
 * @param timeCode The timeCode to be checked.
 * @return True if the attribute has timesampled on key timeCode.
 */
bool hasTimeSample(const PXR_NS::UsdAttribute& attribute, PXR_NS::UsdTimeCode timeCode)
{
    if (timeCode.IsDefault())
        return false;

    std::vector<double> times;
    if (attribute.GetTimeSamples(&times))
    {
        double timeCodeValue = timeCode.GetValue();
        if (round(timeCodeValue) != timeCode.GetValue())
        {
            std::cerr << "Error : Try to identify attribute " << attribute.GetPath().GetText() << " has time sample on a fractinal key frame " << timeCodeValue << std::endl;
            return false;
        }
        return std::find(times.begin(), times.end(), timeCodeValue) != times.end();
    }
    return false;
}

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
TimeSamplesOnLayer getAttributeEffectiveTimeSampleLayerInfo(const PXR_NS::UsdStage& stage, const PXR_NS::UsdAttribute& attr, PXR_NS::SdfLayerRefPtr* outLayer = nullptr)
{
    if (attr.GetNumTimeSamples() == 0)
        return TimeSamplesOnLayer::eNoTimeSamples;

    auto authoringLayer = stage.GetEditTarget().GetLayer();

    bool isOnStrongerLayer = true;
    const pxr::PcpLayerStackPtr& nodeLayers = attr.GetResolveInfo().GetNode().GetLayerStack();
    const pxr::SdfLayerRefPtrVector& layerStack = nodeLayers->GetLayers();
    for (auto layer : layerStack)
    {
        auto attrSpec = layer->GetAttributeAtPath(attr.GetPath());
        if (attrSpec && attrSpec->GetTimeSampleMap().size() > 0)
        {
            if (outLayer)
            {
                *outLayer = layer;
            }
            if (layer == authoringLayer)
            {
                return TimeSamplesOnLayer::eOnCurrentLayer;
            }
            else if (isOnStrongerLayer)
            {
                return TimeSamplesOnLayer::eOnStrongerLayer;
            }
            else
            {
                return TimeSamplesOnLayer::eOnWeakerLayer;
            }
        }
        else
        {
            if (layer == authoringLayer)
            {
                isOnStrongerLayer = false;
            }
        }
    }
    return TimeSamplesOnLayer::eNoTimeSamples;
}

/**
 * Copy TimeSample From Waker Layer.
 *
 * @param Stage  Current Working Stage.
 * @param attribute The attribute to check.
 */
void copyTimeSamplesFromWeakerLayer(PXR_NS::UsdStage& stage, const PXR_NS::UsdAttribute& attr)
{
    PXR_NS::SdfLayerRefPtr outLayer;
    if (getAttributeEffectiveTimeSampleLayerInfo(stage, attr, &outLayer) == TimeSamplesOnLayer::eOnWeakerLayer)
    {
        PXR_NS::SdfTimeSampleMap timesamples;
        if (attr.GetMetadata(PXR_NS::TfToken("timeSamples"), &timesamples))
        {
            attr.SetMetadata(PXR_NS::TfToken("timeSamples"), timesamples);
        }
    }
}

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
)
{
    using namespace PXR_NS;

    UsdTimeCode setTimeCode = timeCode;
    if (!isTimeSampled(attribute))
    {
        setTimeCode = UsdTimeCode::Default();
    }

    // This is here to prevent the TransformGizmo from writing a translation, rotation and scale on every
    // key where it sets a value. At some point we should revisit the gizmo to simplify the logic, and
    // start setting only the transform value the user intends.
    if (skipEqualSetForTimeSample)
    {
        if (!setTimeCode.IsDefault() && !hasTimeSample(attribute, setTimeCode))
        {
            ValueType value;
            bool result = attribute.Get(&value, timeCode);
            if (result && GfIsClose(value, val, 1e-6))
            {
                return false;
            }
        }
    }

    // if the prim is created by Kit on session layer, switch EditTarget to session layer instead
    auto stage = attribute.GetStage();
    auto sessionLayer = stage->GetSessionLayer();
    auto primSpec = sessionLayer->GetPrimAtPath(attribute.GetPrimPath());
    if (primSpec && SdfIsDefiningSpecifier(primSpec->GetSpecifier()))
    {
        ScopedLayerEdit scopedSessionLayerEdit(stage, sessionLayer);
        return attribute.Set(val);
    }
    else
    {
        copyTimeSamplesFromWeakerLayer(*stage, attribute);
        return attribute.Set(val, setTimeCode);
    }
}

template <class HalfType, class FloatType, class DoubleType, class ValueType>
bool setValueWithPrecision(
    PXR_NS::UsdGeomXformOp& xformOp,
    const ValueType& value,
    PXR_NS::UsdTimeCode timeCode = PXR_NS::UsdTimeCode::Default(),
    bool skipEqualSetForTimeSample = false)
{
    using namespace PXR_NS;
    
    switch (xformOp.GetPrecision())
    {
    case UsdGeomXformOp::PrecisionHalf:
        return setAttribute(xformOp.GetAttr(), HalfType(FloatType(value)), timeCode, skipEqualSetForTimeSample);
    case UsdGeomXformOp::PrecisionFloat:
        return setAttribute(xformOp.GetAttr(), FloatType(value), timeCode, skipEqualSetForTimeSample);
    case UsdGeomXformOp::PrecisionDouble:
        return setAttribute(xformOp.GetAttr(), DoubleType(value), timeCode, skipEqualSetForTimeSample);
    }
    return false;
}

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
    PXR_NS::GfVec3d &scale,
    PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()
)
{
    using namespace PXR_NS;

    bool resetXformStack = false;
    UsdGeomXformable xform(prim);

    std::vector<UsdGeomXformOp> orderedXformOps = xform.GetOrderedXformOps(&resetXformStack);

    bool seenScale = false;
    uint32_t seenRotation = 0; // use a counter here, because euler angle can show up as individual xformOp.
    bool seenAxes[3] = { false, false, false };
    bool seenTranslation = false;

    // default values
    scale.Set(1.0, 1.0, 1.0);
    rotation.Set(0.0, 0.0, 0.0);
    rotationOrder.Set(-1, -1, -1); // placeholder
    translation.Set(0.0, 0.0, 0.0);

    for (auto it = orderedXformOps.rbegin(); it != orderedXformOps.rend(); ++it)
    {
        const UsdGeomXformOp& xformOp = *it;

        if (xformOp.IsInverseOp())
        {
            continue;
        }

        const UsdGeomXformOp::Type opType = xformOp.GetOpType();
        const UsdGeomXformOp::Precision precision = xformOp.GetPrecision();

        if (opType == UsdGeomXformOp::TypeTransform)
        {
            seenScale = true;
            seenRotation = 3;
            seenTranslation = true;
            GfMatrix4d mtx = xformOp.GetOpTransform(time);

            GfMatrix4d rotMat(1.0);
            GfMatrix4d scaleOrientMatUnused, perspMatUnused;
            mtx.Factor(&scaleOrientMatUnused, &scale, &rotMat, &translation, &perspMatUnused);

            // By default decompose as XYZ order (make it an option?)
            GfVec3d decompRot =
                rotMat.ExtractRotation().Decompose(GfVec3d::ZAxis(), GfVec3d::YAxis(), GfVec3d::XAxis());
            rotation = { decompRot[2], decompRot[1], decompRot[0] };
            rotationOrder = { 0, 1, 2 };

            break;
        }

        if (!seenScale)
        {
            if (opType == UsdGeomXformOp::TypeScale)
            {
                if (seenRotation || seenTranslation)
                {
                    std::cerr << "WARNING: Incompatible xformOpOrder, rotation or translation applied before scale." << std::endl;
                }

                seenScale = true;
                xformOp.GetAs<>(&scale, time);
            }
        }

        if (seenRotation != 3)
        {
            if (opType >= UsdGeomXformOp::TypeRotateXYZ && opType <= UsdGeomXformOp::TypeRotateZYX)
            {
                if (seenTranslation || seenRotation != 0)
                {
                    std::cerr << "WARNING: Incompatible xformOpOrder, translation applied before rotation or too many rotation ops." << std::endl;
                }

                seenRotation = 3;
                xformOp.GetAs<>(&rotation, time);

                static const GfVec3i kRotationOrder[] = {
                    { 0, 1, 2 }, // XYZ
                    { 0, 2, 1 }, // XZY
                    { 1, 0, 2 }, // YXZ
                    { 1, 2, 0 }, // YZX
                    { 2, 0, 1 }, // ZXY
                    { 2, 1, 0 }, // ZYX
                };

                rotationOrder = kRotationOrder[opType - UsdGeomXformOp::TypeRotateXYZ];
            }
            else if (opType >= UsdGeomXformOp::TypeRotateX && opType <= UsdGeomXformOp::TypeRotateZ)
            {
                if (seenTranslation || seenRotation > 3)
                {
                    std::cerr << "WARNING: Incompatible xformOpOrder, too many single axis rotation ops." << std::endl;
                }

                // Set rotation order based on individual axis order
                rotationOrder[seenRotation++] = opType - UsdGeomXformOp::TypeRotateX;
                seenAxes[opType - UsdGeomXformOp::TypeRotateX] = true;

                double angle = 0.0;
                xformOp.GetAs<>(&angle, time);
                rotation[opType - UsdGeomXformOp::TypeRotateX] = angle;
            }
            else if (opType == UsdGeomXformOp::TypeOrient)
            {
                if (seenTranslation || seenRotation != 0)
                {
                    std::cerr << "WARNING: Incompatible xformOpOrder, translation applied before rotation or too many rotation ops." << std::endl;
                }

                seenRotation = 3;
                GfRotation rot;

                // GetAs cannot convert between Quath, Quatf and Quatd
                switch (precision)
                {
                case PXR_NS::UsdGeomXformOp::PrecisionHalf:
                {
                    rot = getRotationFromXformOpOrient<GfQuath>(xformOp, time);
                    break;
                }
                case PXR_NS::UsdGeomXformOp::PrecisionFloat:
                {
                    rot = getRotationFromXformOpOrient<GfQuatf>(xformOp, time);
                    break;
                }
                case PXR_NS::UsdGeomXformOp::PrecisionDouble:
                {
                    rot = getRotationFromXformOpOrient<GfQuatd>(xformOp, time);
                    break;
                }
                default:
                    break;
                }

                // By default decompose as XYZ order (make it an option?)
                GfVec3d decompRot = rot.Decompose(GfVec3d::ZAxis(), GfVec3d::YAxis(), GfVec3d::XAxis());
                rotation = { decompRot[2], decompRot[1], decompRot[0] };
                rotationOrder = { 0, 1, 2 };
            }
        }

        if (!seenTranslation)
        {
            // Do not get translation from pivot
            if (opType == UsdGeomXformOp::TypeTranslate && !xformOp.HasSuffix(PXR_NS::TfToken("pivot")))
            {
                seenTranslation = true;
                xformOp.GetAs<>(&translation, time);
            }
        }
    }

    if (seenRotation == 0)
    {
        // If we did not see any rotation op, assume XYZ
        static GfVec3i gDefaultRotationOrder = { 0, 1, 2 };
        rotationOrder = gDefaultRotationOrder;
        seenRotation = 3;
    }
    else
    {
        // Assign rotation order to missing rotation ops after existing rotation ops
        for (size_t i = 0; i < 3; i++)
        {
            int32_t& order = rotationOrder[i];
            if (order == -1)
            {
                for (int32_t j = 0; j < 3; j++)
                {
                    if (!seenAxes[j])
                    {
                        order = j;
                        seenAxes[j] = true;
                        break;
                    }
                }
            }
        }
    }

    return true;
}

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
)
{
    using namespace PXR_NS;

    // If prim is defined in session layer, we author in session layer.
    UsdEditTarget editTarget = prim.GetStage()->GetEditTarget();
    auto mightDefOnSessionLayer = getLayerIfDefOnSessionOrItsSublayers(prim.GetStage(), prim.GetPath());
    if (mightDefOnSessionLayer)
    {
        editTarget = UsdEditTarget(mightDefOnSessionLayer);
    }

    UsdEditContext editContext(prim.GetStage(), editTarget);
    std::unique_ptr<SdfChangeBlock> localChangeBlock;
    std::unique_ptr<SdfChangeBlock>& changeBlock =
        (parentChangeBlock != nullptr) ? *parentChangeBlock : localChangeBlock;
    if (!changeBlock.get())
    {
        changeBlock.reset(new SdfChangeBlock());
    }

    const GfMatrix4d mtx = constructTransformMatrixFromSRT(translation, rotationEuler, rotationOrder, scale);

    UsdGeomXformable xform(prim);

    bool resetXFormStack;
    auto xformOps = xform.GetOrderedXformOps(&resetXFormStack);
    PXR_NS::VtTokenArray xformOpOrders;
    xform.GetXformOpOrderAttr().Get(&xformOpOrders);
    bool foundTransformOp = false;
    bool success = true;
    const GfMatrix4d innerMtx = findInnerTransform(prim, mtx, foundTransformOp, timecode, skipEqualSetForTimeSample);

    for (auto& xformOp : xformOps)
    {
        // Found transform op, trying to set its value
        if (xformOp.GetOpType() == UsdGeomXformOp::TypeTransform)
        {
            foundTransformOp = true;
            success &= ::setAttribute(xformOp.GetAttr(), innerMtx, timecode, skipEqualSetForTimeSample);
        }
    }

    // If transformOp is not found, make individual xformOp or reuse old ones.
    if (!foundTransformOp)
    {
        // Don't use UsdGeomXformCommonAPI. It can only manipulate a very limited subset of xformOpOrder
        // combinations Do it manually as non-destructively as possible
        UsdGeomXformOp xformOp;
        std::vector<UsdGeomXformOp> newXformOps;

        auto findOrAdd = [&xformOps, &xformOpOrders, &xform, &changeBlock](
                                UsdGeomXformOp::Type xformOpType, UsdGeomXformOp& outXformOp, bool createIfNotExist,
                                UsdGeomXformOp::Precision& precision, TfToken const& opSuffix = TfToken()) {
            for (auto xformOp : xformOps)
            {
                if (xformOp.GetOpType() == xformOpType)
                {
                    // To differentiate translate and translate:pivot
                    std::string expectedOpName = UsdGeomXformOp::GetOpName(xformOpType, opSuffix);
                    std::string opName = xformOp.GetOpName().GetString();
                    if (opName == expectedOpName)
                    {
                        precision = xformOp.GetPrecision();
                        outXformOp = xformOp;
                        return true;
                    }
                }
            }

            if (createIfNotExist)
            {
                // It is not safe to create new xformOps inside of SdfChangeBlocks, since
                // new attribute creation via anything above Sdf API requires the PcpCache
                // to be up to date. Flush the current change block before creating
                // the new xformOp.
                changeBlock.reset(nullptr);
                if (std::find(xformOpOrders.begin(), xformOpOrders.end(),
                                UsdGeomXformOp::GetOpName(xformOpType, opSuffix)) == xformOpOrders.end())
                    outXformOp = xform.AddXformOp(xformOpType, precision, opSuffix);
                else
                {
                    // Sometimes XformOp attributes and XformOpOrder don't match. GetOrderedXformOps() considers
                    // both XformOp attributes and XformOpOrder. But AddXformOp() considers only XformOpOrder. So we
                    // need to fix it here.
                    auto opAttr = xform.GetPrim().CreateAttribute(
                        UsdGeomXformOp::GetOpName(xformOpType, opSuffix),
                        UsdGeomXformOp::GetValueTypeName(xformOpType, precision), false);
                    outXformOp = UsdGeomXformOp(opAttr);
                }

                // Create a new change block to batch the subsequent authoring operations
                // where possible.
                changeBlock.reset(new SdfChangeBlock());
                // Creation may have failed for a variety of reasons (including instanceable=True)
                return static_cast<bool>(outXformOp);
            }
            return false;
        };

        auto getFirstRotateOpType = [&xformOps](UsdGeomXformOp::Precision& precision) {
            for (auto xformOp : xformOps)
            {
                if (xformOp.GetOpType() >= UsdGeomXformOp::Type::TypeRotateX &&
                    xformOp.GetOpType() <= UsdGeomXformOp::Type::TypeOrient)
                {
                    precision = xformOp.GetPrecision();
                    return xformOp.GetOpType();
                }
            }
            return UsdGeomXformOp::Type::TypeInvalid;
        };

        auto setEulerValue = [&rotationEuler, &findOrAdd, &newXformOps](
                                    UsdGeomXformOp::Type rotationType, UsdGeomXformOp::Precision precision,
                                    UsdTimeCode timecode, bool skipEqualSetForTimeSample) {
            bool ret = false;
            UsdGeomXformOp xformOp;
            if (findOrAdd(rotationType, xformOp, true, precision))
            {
                ret = setValueWithPrecision<GfVec3h, GfVec3f, GfVec3d, GfVec3d>(
                    xformOp, rotationEuler, timecode, skipEqualSetForTimeSample);
                newXformOps.push_back(xformOp);
            }
            return ret;
        };

        // Set translation
        UsdGeomXformOp::Precision precision = UsdGeomXformOp::PrecisionDouble;
        if (findOrAdd(UsdGeomXformOp::TypeTranslate, xformOp, true, precision))
        {
            success &= setValueWithPrecision<GfVec3h, GfVec3f, GfVec3d, GfVec3d>(
                xformOp, translation, timecode, skipEqualSetForTimeSample);
            newXformOps.push_back(xformOp);
        }

        // Set pivot
        precision = UsdGeomXformOp::PrecisionFloat;
        UsdGeomXformOp pivotOp;
        GfVec3d pivotValue(0., 0., 0.);
        const bool hasPivot = findOrAdd(UsdGeomXformOp::TypeTranslate, pivotOp, false, precision, TfToken("pivot"));
        if (hasPivot)
        {
            newXformOps.push_back(pivotOp);
        }

        // Set rotation
        precision = UsdGeomXformOp::PrecisionFloat;
        auto firstRotateOpType = getFirstRotateOpType(precision);

        struct HashFn
        {
            size_t operator()(const GfVec3i& vec) const
            {
                return hash_value(vec);
            }
        };

        static const std::unordered_map<GfVec3i, UsdGeomXformOp::Type, HashFn> kRotationOrderToTypeMap{
            { { 0, 1, 2 }, UsdGeomXformOp::TypeRotateXYZ }, { { 0, 2, 1 }, UsdGeomXformOp::TypeRotateXZY },
            { { 1, 0, 2 }, UsdGeomXformOp::TypeRotateYXZ }, { { 1, 2, 0 }, UsdGeomXformOp::TypeRotateYZX },
            { { 2, 0, 1 }, UsdGeomXformOp::TypeRotateZXY }, { { 2, 1, 0 }, UsdGeomXformOp::TypeRotateZYX },
        };

        if (firstRotateOpType == UsdGeomXformOp::TypeInvalid)
        {
            // If no rotation was defined on the prim, use rotationOrder as default
            const auto orderEntry = kRotationOrderToTypeMap.find(rotationOrder);
            if (orderEntry != kRotationOrderToTypeMap.end())
            {
                firstRotateOpType = orderEntry->second;
            }
        }

        switch (firstRotateOpType)
        {
        case UsdGeomXformOp::TypeRotateX:
        case UsdGeomXformOp::TypeRotateY:
        case UsdGeomXformOp::TypeRotateZ:
        {
            // Add in reverse order
            for (int32_t i = 2; i >= 0; i--)
            {
                size_t axis = rotationOrder[i];
                if (findOrAdd(static_cast<UsdGeomXformOp::Type>(UsdGeomXformOp::TypeRotateX + axis), xformOp, true,
                                precision))
                {
                    success &= setValueWithPrecision<GfHalf, float, double, double>(
                        xformOp, rotationEuler[axis], timecode, skipEqualSetForTimeSample);
                    newXformOps.push_back(xformOp);
                }
            }

            break;
        }
        case UsdGeomXformOp::TypeRotateXYZ:
        case UsdGeomXformOp::TypeRotateXZY:
        case UsdGeomXformOp::TypeRotateYXZ:
        case UsdGeomXformOp::TypeRotateYZX:
        case UsdGeomXformOp::TypeRotateZXY:
        case UsdGeomXformOp::TypeRotateZYX:
        {
            UsdGeomXformOp::Type providedRotationOrder = firstRotateOpType;
            const auto orderEntry = kRotationOrderToTypeMap.find(rotationOrder);
            if (orderEntry != kRotationOrderToTypeMap.end())
            {
                providedRotationOrder = orderEntry->second;
            }

            if (providedRotationOrder != firstRotateOpType)
            {
                std::cerr << "WARNING: Existing rotation order on prim" << prim.GetPrimPath().GetText() 
                    << " is different than desired (" << rotationOrder[0] << "," << rotationOrder[1] << "," << rotationOrder[2]
                    << "), overriding..." << std::endl;
            }
            success &= setEulerValue(providedRotationOrder, precision, timecode, skipEqualSetForTimeSample);
            break;
        }
        case UsdGeomXformOp::TypeOrient:
            if (findOrAdd(UsdGeomXformOp::TypeOrient, xformOp, true, precision))
            {
                static const GfVec3d kAxes[] = { GfVec3d::XAxis(), GfVec3d::YAxis(), GfVec3d::ZAxis() };
                GfRotation rotation = GfRotation(kAxes[rotationOrder[0]], rotationEuler[rotationOrder[0]]) *
                                        GfRotation(kAxes[rotationOrder[1]], rotationEuler[rotationOrder[1]]) *
                                        GfRotation(kAxes[rotationOrder[2]], rotationEuler[rotationOrder[2]]);

                success &= setValueWithPrecision<GfQuath, GfQuatf, GfQuatd, GfQuatd>(
                    xformOp, rotation.GetQuat(), timecode, skipEqualSetForTimeSample);
                newXformOps.push_back(xformOp);
            }
            break;
        default:
            std::cerr << "WARNING: Failed to determine rotation order" << std::endl;
        }

        // Set scale
        precision = UsdGeomXformOp::PrecisionFloat;
        if (findOrAdd(UsdGeomXformOp::TypeScale, xformOp, true, precision))
        {
            success &= setValueWithPrecision<GfVec3h, GfVec3f, GfVec3d, GfVec3d>(
                xformOp, scale, timecode, skipEqualSetForTimeSample);
            newXformOps.push_back(xformOp);
        }

        // Set inverse pivot
        if (hasPivot)
        {
            // Assume the last xformOps is the pivot
            newXformOps.push_back(xformOps.back());
        }

        success &= xform.SetXformOpOrder(newXformOps, resetXFormStack);
    }
    return success;
}

} // namespace xformUtils
