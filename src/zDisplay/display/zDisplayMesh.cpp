#include <zspace/zDisplay/display/zDisplayMesh.h>
#include <src/zDisplay/display/internal/zDisplayMeshImpl.h>

#include <utility>

namespace zSpace
{
	ZSPACE_INLINE zDisplayMesh::zDisplayMesh()
		: impl_(std::make_unique<Impl>()) {}

	ZSPACE_INLINE zDisplayMesh::~zDisplayMesh() = default;
	ZSPACE_INLINE zDisplayMesh::zDisplayMesh(const zDisplayMesh& other)
		: impl_(std::make_unique<Impl>(*other.impl_)) {}
	ZSPACE_INLINE zDisplayMesh::zDisplayMesh(zDisplayMesh&& other) noexcept = default;
	ZSPACE_INLINE zDisplayMesh& zDisplayMesh::operator=(const zDisplayMesh& other)
	{
		if (this != &other) *impl_ = *other.impl_;
		return *this;
	}
	ZSPACE_INLINE zDisplayMesh& zDisplayMesh::operator=(zDisplayMesh&& other) noexcept = default;

	ZSPACE_INLINE void zDisplayMesh::setDisplayObject(bool visible) { setObjectVisible(visible); }
	ZSPACE_INLINE void zDisplayMesh::setDisplayTransform(bool visible) { setTransformVisible(visible); }
	ZSPACE_INLINE void zDisplayMesh::setObjectVisible(bool visible) { impl_->objectVisible = visible; }
	ZSPACE_INLINE void zDisplayMesh::setTransformVisible(bool visible) { impl_->transformVisible = visible; }
	ZSPACE_INLINE void zDisplayMesh::setDisplayElements(bool vertices, bool edges, bool faces)
	{
		setElements(vertices, edges, faces);
	}
	ZSPACE_INLINE void zDisplayMesh::setDisplayElementIds(bool vertices, bool edges, bool faces)
	{
		setElementIds(vertices, edges, faces);
	}
	ZSPACE_INLINE void zDisplayMesh::setDisplayVertices(bool visible) { setVerticesVisible(visible); }
	ZSPACE_INLINE void zDisplayMesh::setDisplayEdges(bool visible) { setEdgesVisible(visible); }
	ZSPACE_INLINE void zDisplayMesh::setDisplayFaces(bool visible) { setFacesVisible(visible); }
	ZSPACE_INLINE void zDisplayMesh::setDisplayDihedralEdges(bool visible, double threshold)
	{
		setDihedralEdgesVisible(visible, threshold);
	}
	ZSPACE_INLINE void zDisplayMesh::setDisplayVertexNormals(bool visible, double scale)
	{
		setVertexNormalsVisible(visible, scale);
	}
	ZSPACE_INLINE void zDisplayMesh::setDisplayFaceNormals(bool visible, double scale)
	{
		setFaceNormalsVisible(visible, scale);
	}
	ZSPACE_INLINE void zDisplayMesh::setElements(bool vertices, bool edges, bool faces)
	{
		impl_->verticesVisible = vertices;
		impl_->edgesVisible = edges;
		impl_->facesVisible = faces;
	}
	ZSPACE_INLINE void zDisplayMesh::setElementIds(bool vertices, bool edges, bool faces)
	{
		impl_->vertexIdsVisible = vertices;
		impl_->edgeIdsVisible = edges;
		impl_->faceIdsVisible = faces;
	}
	ZSPACE_INLINE void zDisplayMesh::setVerticesVisible(bool visible) { impl_->verticesVisible = visible; }
	ZSPACE_INLINE void zDisplayMesh::setEdgesVisible(bool visible) { impl_->edgesVisible = visible; }
	ZSPACE_INLINE void zDisplayMesh::setFacesVisible(bool visible) { impl_->facesVisible = visible; }
	ZSPACE_INLINE void zDisplayMesh::setDihedralEdgesVisible(bool visible, double threshold)
	{
		impl_->dihedralEdgesVisible = visible;
		impl_->dihedralAngleThreshold = threshold;
		if (visible) impl_->edgesVisible = false;
	}
	ZSPACE_INLINE void zDisplayMesh::setVertexNormalsVisible(bool visible, double scale)
	{
		impl_->vertexNormalsVisible = visible;
		impl_->normalScale = scale;
	}
	ZSPACE_INLINE void zDisplayMesh::setFaceNormalsVisible(bool visible, double scale)
	{
		impl_->faceNormalsVisible = visible;
		impl_->normalScale = scale;
	}
	ZSPACE_INLINE void zDisplayMesh::setFaceCenters(const std::vector<zVector>& centers) { impl_->faceCenters = centers; }
	ZSPACE_INLINE void zDisplayMesh::setEdgeCenters(const std::vector<zVector>& centers) { impl_->edgeCenters = centers; }
	ZSPACE_INLINE void zDisplayMesh::setDihedralAngles(const std::vector<double>& angles) { impl_->dihedralAngles = angles; }

	ZSPACE_INLINE bool zDisplayMesh::objectVisible() const { return impl_->objectVisible; }
	ZSPACE_INLINE bool zDisplayMesh::transformVisible() const { return impl_->transformVisible; }
	ZSPACE_INLINE bool zDisplayMesh::getDisplayObject() const { return objectVisible(); }
	ZSPACE_INLINE bool zDisplayMesh::getDisplayTransform() const { return transformVisible(); }
	ZSPACE_INLINE bool zDisplayMesh::verticesVisible() const { return impl_->verticesVisible; }
	ZSPACE_INLINE bool zDisplayMesh::edgesVisible() const { return impl_->edgesVisible; }
	ZSPACE_INLINE bool zDisplayMesh::facesVisible() const { return impl_->facesVisible; }
	ZSPACE_INLINE bool zDisplayMesh::vertexIdsVisible() const { return impl_->vertexIdsVisible; }
	ZSPACE_INLINE bool zDisplayMesh::edgeIdsVisible() const { return impl_->edgeIdsVisible; }
	ZSPACE_INLINE bool zDisplayMesh::faceIdsVisible() const { return impl_->faceIdsVisible; }
	ZSPACE_INLINE bool zDisplayMesh::dihedralEdgesVisible() const { return impl_->dihedralEdgesVisible; }
	ZSPACE_INLINE bool zDisplayMesh::vertexNormalsVisible() const { return impl_->vertexNormalsVisible; }
	ZSPACE_INLINE bool zDisplayMesh::faceNormalsVisible() const { return impl_->faceNormalsVisible; }
	ZSPACE_INLINE double zDisplayMesh::dihedralAngleThreshold() const { return impl_->dihedralAngleThreshold; }
	ZSPACE_INLINE double zDisplayMesh::normalScale() const { return impl_->normalScale; }
	ZSPACE_INLINE const std::vector<zVector>& zDisplayMesh::faceCenters() const { return impl_->faceCenters; }
	ZSPACE_INLINE const std::vector<zVector>& zDisplayMesh::edgeCenters() const { return impl_->edgeCenters; }
	ZSPACE_INLINE const std::vector<double>& zDisplayMesh::dihedralAngles() const { return impl_->dihedralAngles; }
}
