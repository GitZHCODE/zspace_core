#pragma once

#if defined(ZSPACE_MAYA_INTEROP)

//--------------------------
//---- zMayaFNSETS
//--------------------------
#include <headers/zInterOp/functionSets/zMayaFnMesh.h>
#include <headers/zInterOp/functionSets/zMayaFnGraph.h>

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
#include <headers/zInterOp/objects/zObjNurbsCurve.h>

//--------------------------
//---- zRhinoFNSETS
//--------------------------
#include <headers/zInterOp/functionSets/zRhinoFnMesh.h>
#include <headers/zInterOp/functionSets/zRhinoFnGraph.h>

#include <headers/zInterOp/functionSets/zFnNurbsCurve.h>

#endif

#if defined(ZSPACE_MAYA_INTEROP)  && defined(ZSPACE_RHINO_INTEROP)

//--------------------------
//---- zInterOp
//--------------------------

#include <headers/zInterOp/interOp/zIOMesh.h>
#include <headers/zInterOp/interOp/zIOMeshSurface.h>
#include <headers/zInterOp/interOp/zIONurbsCurve.h>
#include <headers/zInterOp/interOp/zIONurbsSurface.h>


#endif