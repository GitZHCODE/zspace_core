#include <zspace/interop.h>
#include <zspace/zInterOp/objects/zObjectArc.h>
#include <zspace/zInterOp/objects/zObjectNurbsCurve.h>
#include <zspace/zInterOp/objects/zObjectPlane.h>

static_assert(sizeof(zSpace::zObjectArc*) == sizeof(void*));
static_assert(sizeof(zSpace::zObjectNurbsCurve*) == sizeof(void*));
static_assert(sizeof(zSpace::zObjectPlane*) == sizeof(void*));
