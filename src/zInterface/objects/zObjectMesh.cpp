#include <zspace/zInterface/objects/zObjectMesh.h>
#include <src/zInterface/objects/zMeshObjectStorage.h>

namespace zSpace
{
	ZSPACE_INLINE zObjectMesh::zObjectMesh()
		: impl(std::make_unique<Impl>())
	{
	}

	ZSPACE_INLINE zObjectMesh::zObjectMesh(const zObjectMesh& other)
		: zObject(other),
		  impl(std::make_unique<Impl>(*other.impl))
	{
	}

	ZSPACE_INLINE zObjectMesh::zObjectMesh(zObjectMesh&& other) noexcept = default;

	ZSPACE_INLINE zObjectMesh& zObjectMesh::operator=(const zObjectMesh& other)
	{
		if (this == &other) return *this;
		zObject::operator=(other);
		impl = std::make_unique<Impl>(*other.impl);
		return *this;
	}

	ZSPACE_INLINE zObjectMesh& zObjectMesh::operator=(zObjectMesh&& other) noexcept = default;
	ZSPACE_INLINE zObjectMesh::~zObjectMesh() = default;

}
