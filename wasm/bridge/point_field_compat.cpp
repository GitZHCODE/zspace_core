// Clang requires these specializations before the upstream definitions call them.
// Core owns this optional WASM field compatibility wrapper.
#include <zspace/zInterface/functionsets/zFnPointField.h>
namespace zSpace {
template<>
	ZSPACE_INLINE vector<zVector> zFnPointFieldBase<zScalar>::getGradients(float epsilon)
	;
template<>
	ZSPACE_INLINE void zFnPointFieldBase<zScalar>::setFieldValues(zScalarArray& fValues)
	;
template<>
	ZSPACE_INLINE void zFnPointFieldBase<zVector>::setFieldValues(vector<zVector>& fValues)
	;
template<>
	ZSPACE_INLINE void zFnPointFieldBase<zScalar>::normliseValues(zScalarArray& fieldValues)
	;
template<>
	ZSPACE_INLINE void zFnPointFieldBase<zVector>::normliseValues(vector<zVector>& fieldValues)
	;
template<>
	ZSPACE_INLINE void zFnPointFieldBase<zScalar>::updateColors()
	;
}
#include <src/zInterface/functionsets/zFnPointField.cpp>
