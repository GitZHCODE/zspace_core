#ifndef ZSPACE_LEGACY_ZOBJ_MESHFIELD_H
#define ZSPACE_LEGACY_ZOBJ_MESHFIELD_H

#pragma once

#include <zspace/zInterface/objects/zObjectMeshField.h>

namespace zSpace
{
	template<typename T>
	using zObjMeshField = zObjectMeshField<T>;

	using zObjMeshScalarField = zObjectMeshScalarField;
	using zObjMeshVectorField = zObjectMeshVectorField;
	using zObjMeshScalarFieldArray = zObjectMeshScalarFieldArray;
	using zObjMeshVectorFieldArray = zObjectMeshVectorFieldArray;
}

#endif
