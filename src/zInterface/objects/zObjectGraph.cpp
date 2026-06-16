#include <zspace/zInterface/objects/zObjectGraph.h>
#include <src/zInterface/objects/zGraphObjectStorage.h>

namespace zSpace
{
	ZSPACE_INLINE zObjectGraph::zObjectGraph()
		: impl(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zObjectGraph::zObjectGraph(const zObjectGraph& other)
		: zObject(other),
		  impl(std::make_unique<Impl>(*other.impl))
	{
	}

	ZSPACE_INLINE zObjectGraph::zObjectGraph(zObjectGraph&& other) noexcept = default;

	ZSPACE_INLINE zObjectGraph& zObjectGraph::operator=(const zObjectGraph& other)
	{
		if (this == &other) return *this;
		zObject::operator=(other);
		impl = std::make_unique<Impl>(*other.impl);
		return *this;
	}

	ZSPACE_INLINE zObjectGraph& zObjectGraph::operator=(zObjectGraph&& other) noexcept = default;
	ZSPACE_INLINE zObjectGraph::~zObjectGraph() = default;

}
