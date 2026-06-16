#include <zspace/zDisplay/display/zDisplayGraph.h>
#include <src/zDisplay/display/internal/zDisplayGraphImpl.h>

#include <utility>

namespace zSpace
{
	ZSPACE_INLINE zDisplayGraph::zDisplayGraph()
		: impl_(std::make_unique<Impl>()) {}

	ZSPACE_INLINE zDisplayGraph::~zDisplayGraph() = default;
	ZSPACE_INLINE zDisplayGraph::zDisplayGraph(const zDisplayGraph& other)
		: impl_(std::make_unique<Impl>(*other.impl_)) {}
	ZSPACE_INLINE zDisplayGraph::zDisplayGraph(zDisplayGraph&& other) noexcept = default;
	ZSPACE_INLINE zDisplayGraph& zDisplayGraph::operator=(const zDisplayGraph& other)
	{
		if (this != &other) *impl_ = *other.impl_;
		return *this;
	}
	ZSPACE_INLINE zDisplayGraph& zDisplayGraph::operator=(zDisplayGraph&& other) noexcept = default;

	ZSPACE_INLINE void zDisplayGraph::setObjectVisible(bool visible) { impl_->objectVisible = visible; }
	ZSPACE_INLINE void zDisplayGraph::setTransformVisible(bool visible) { impl_->transformVisible = visible; }
	ZSPACE_INLINE void zDisplayGraph::setElements(bool vertices, bool edges)
	{
		impl_->verticesVisible = vertices;
		impl_->edgesVisible = edges;
	}
	ZSPACE_INLINE void zDisplayGraph::setElementIds(bool vertices, bool edges)
	{
		impl_->vertexIdsVisible = vertices;
		impl_->edgeIdsVisible = edges;
	}
	ZSPACE_INLINE void zDisplayGraph::setVerticesVisible(bool visible) { impl_->verticesVisible = visible; }
	ZSPACE_INLINE void zDisplayGraph::setEdgesVisible(bool visible) { impl_->edgesVisible = visible; }
	ZSPACE_INLINE void zDisplayGraph::setEdgeCenters(const std::vector<zVector>& centers) { impl_->edgeCenters = centers; }

	ZSPACE_INLINE bool zDisplayGraph::objectVisible() const { return impl_->objectVisible; }
	ZSPACE_INLINE bool zDisplayGraph::transformVisible() const { return impl_->transformVisible; }
	ZSPACE_INLINE bool zDisplayGraph::verticesVisible() const { return impl_->verticesVisible; }
	ZSPACE_INLINE bool zDisplayGraph::edgesVisible() const { return impl_->edgesVisible; }
	ZSPACE_INLINE bool zDisplayGraph::vertexIdsVisible() const { return impl_->vertexIdsVisible; }
	ZSPACE_INLINE bool zDisplayGraph::edgeIdsVisible() const { return impl_->edgeIdsVisible; }
	ZSPACE_INLINE const std::vector<zVector>& zDisplayGraph::edgeCenters() const { return impl_->edgeCenters; }
}
