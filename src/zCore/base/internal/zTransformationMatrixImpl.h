#ifndef ZSPACE_TRANSFORMATION_MATRIX_IMPL_H
#define ZSPACE_TRANSFORMATION_MATRIX_IMPL_H

#include <zspace/zCore/base/zTransformationMatrix.h>

namespace zSpace
{
	class zTransformationMatrix::Impl
	{
	public:
		zTransform Transform;
		zTransform P;
		zTransform S;
		zTransform T;
		zTransform R;
		zTransform Rx;
		zTransform Ry;
		zTransform Rz;
		zFloat4 rotation;
		zFloat4 scale;
		zFloat4 translation;
		zFloat4 pivot;
	};
}

#endif
