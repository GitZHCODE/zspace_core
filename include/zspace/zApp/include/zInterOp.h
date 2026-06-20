#pragma once

#if defined(ZSPACE_MAYA_INTEROP)

//--------------------------
//---- zMayaFNSETS
//--------------------------
#include <zspace/zInterOp/functionsets/zMayaFnMesh.h>
#include <zspace/zInterOp/functionsets/zMayaFnGraph.h>

#endif

#if defined(ZSPACE_RHINO_INTEROP)

//--------------------------
//---- windows include
//--------------------------
#include<Unknwnbase.h>
#include<commdlg.h>

//--------------------------
//---- zRhinoObjects
//--------------------------
#include <zspace/zInterOp/objects/zObjectNurbsCurve.h>

//--------------------------
//---- zRhinoFNSETS
//--------------------------
#include <zspace/zInterOp/functionsets/zRhinoFnMesh.h>
#include <zspace/zInterOp/functionsets/zRhinoFnGraph.h>

#include <zspace/zInterOp/functionsets/zFnNurbsCurve.h>
#include <zspace/zInterOp/functionsets/zFnPlane.h>


#endif

#if defined(ZSPACE_MAYA_INTEROP)  && defined(ZSPACE_RHINO_INTEROP)

//--------------------------
//---- zInterOp
//--------------------------

#include <zspace/zInterOp/converters/zIOMesh.h>
#include <zspace/zInterOp/converters/zIOMeshSurface.h>
#include <zspace/zInterOp/converters/zIONurbsCurve.h>
#include <zspace/zInterOp/converters/zIONurbsSurface.h>


#endif

#if defined(ZSPACE_USD_INTEROP) 

//--------------------------
//---- zInterOp
//--------------------------



#endif