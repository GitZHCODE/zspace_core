#ifndef ZSPACE_INTEROP_UMBRELLA_H
#define ZSPACE_INTEROP_UMBRELLA_H

#pragma once

#include <zspace/interface.h>
#include <zspace/io.h>
#include <zspace/zInterOp/serialization/zGeometryJson.h>

#if defined(ZSPACE_RHINO_INTEROP)
#include <zspace/zInterOp/core/zRhinoCore.h>
#include <zspace/zInterOp/objects/zObjectArc.h>
#include <zspace/zInterOp/objects/zObjectNurbsCurve.h>
#include <zspace/zInterOp/objects/zObjectPlane.h>
#include <zspace/zInterOp/functionsets/zFnArc.h>
#include <zspace/zInterOp/functionsets/zFnNurbsCurve.h>
#include <zspace/zInterOp/functionsets/zFnPlane.h>
#include <zspace/zInterOp/functionsets/zRhinoFnGraph.h>
#include <zspace/zInterOp/functionsets/zRhinoFnMesh.h>
#include <zspace/zInterOp/converters/zIOMesh.h>
#include <zspace/zInterOp/converters/zIOMeshSurface.h>
#include <zspace/zInterOp/converters/zIONurbsCurve.h>
#include <zspace/zInterOp/converters/zIONurbsSurface.h>
#endif

#if defined(ZSPACE_MAYA_INTEROP)
#include <zspace/zInterOp/functionsets/zMayaFnGraph.h>
#include <zspace/zInterOp/functionsets/zMayaFnMesh.h>
#endif

#if defined(ZSPACE_UNREAL_INTEROP)
#include <zspace/zInterOp/functionsets/zUnrealFnMesh.h>
#endif

#if defined(ZSPACE_USD_INTEROP)
#include <zspace/zInterOp/core/zOmniCore.h>
#include <zspace/zInterOp/core/omniUtils/LiveSessionConfigFile.h>
#include <zspace/zInterOp/core/omniUtils/LiveSessionInfo.h>
#include <zspace/zInterOp/core/omniUtils/OmniChannel.h>
#include <zspace/zInterOp/core/omniUtils/OmniverseUsdLuxLightCompat.h>
#include <zspace/zInterOp/core/omniUtils/primUtils.h>
#include <zspace/zInterOp/core/omniUtils/xformUtils.h>
#endif

#endif
