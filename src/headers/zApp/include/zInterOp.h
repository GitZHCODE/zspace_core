#pragma once

#if defined(ZSPACE_MAYA_INTEROP)

//--------------------------
//---- zMayaFNSETS
//--------------------------
#include "zInterOp/functionSets/zMayaFnMesh.h"
#include "zInterOp/functionSets/zMayaFnGraph.h"

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
#include "zInterOp/objects/zObjNurbsCurve.h"

//--------------------------
//---- zRhinoFNSETS
//--------------------------
#include "zInterOp/functionSets/zRhinoFnMesh.h"
#include "zInterOp/functionSets/zRhinoFnGraph.h"

#include "zInterOp/functionSets/zFnNurbsCurve.h"

#endif

#if defined(ZSPACE_MAYA_INTEROP)  && defined(ZSPACE_RHINO_INTEROP)

//--------------------------
//---- zInterOp
//--------------------------

#include "zInterOp/interOp/zIOMesh.h"
#include "zInterOp/interOp/zIOMeshSurface.h"
#include "zInterOp/interOp/zIONurbsCurve.h"
#include "zInterOp/interOp/zIONurbsSurface.h"


#endif

#if defined(ZSPACE_USD_INTEROP) 

//--------------------------
//---- zInterOp
//--------------------------



#endif