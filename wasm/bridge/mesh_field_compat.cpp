// Clang requires these specializations before the upstream definitions call them.
// Core owns this optional WASM field compatibility wrapper.
#include <zspace/zInterface/functionsets/zFnMeshField.h>
namespace zSpace {
template<>
	ZSPACE_INLINE vector<zVector> zFnMeshFieldBase<zScalar>::getGradients(float epsilon)
	;
template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::setFieldValues(zScalarArray& fValues, zFieldColorType type, float sdfWidth)
	;
template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::setFieldValues(vector<zVector>& fValues, zFieldColorType type, float sdfWidth)
	;
template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::normliseValues(zScalarArray &fieldValues)
	;
template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zVector>::normliseValues(vector<zVector> &fieldValues)
	;
template<>
	ZSPACE_INLINE void zFnMeshFieldBase<zScalar>::updateColors(zFieldColorType type, float sdfWidth)
	;
template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Polygon(zObjectGraph& inGraphObj, zPoint& p)
	;
template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Circle(zPoint &cen, zPoint &p, float r)
	;
template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Ellipse(zPoint& cen, zPoint& p, float a, float b)
	;
template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Line(zPoint &p, zPoint &v0, zPoint &v1)
	;
template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Triangle(zPoint& p, zPoint& p0, zPoint& p1, zPoint& p2)
	;
template<>
	ZSPACE_INLINE float zFnMeshFieldBase<zScalar>::getScalar_Square(zPoint &p, zVector& cen, zVector &dimensions)
	;
template<>
	ZSPACE_INLINE double zFnMeshFieldBase<zScalar>::getScalar_Trapezoid(zPoint &p, float &r1, float &r2, float &he)
	;
}
#include <src/zInterface/functionsets/zFnMeshField.cpp>
