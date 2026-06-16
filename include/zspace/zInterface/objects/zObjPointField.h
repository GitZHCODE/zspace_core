#ifndef ZSPACE_LEGACY_ZOBJ_POINTFIELD_H
#define ZSPACE_LEGACY_ZOBJ_POINTFIELD_H

#pragma once

#include <zspace/zInterface/objects/zObjectPointField.h>

namespace zSpace
{
	template<typename T>
	using zObjPointField = zObjectPointField<T>;

	using zObjPointScalarField = zObjectPointScalarField;
	using zObjPointVectorField = zObjectPointVectorField;
	using zObjPointScalarFieldArray = zObjectPointScalarFieldArray;
	using zObjPointVectorFieldArray = zObjectPointVectorFieldArray;
}

#endif
