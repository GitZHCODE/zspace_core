#include <zspace/zInterface/iterators/zItPointCloud.h>
#include <src/zInterface/objects/internal/zObjPointCloudImpl.h>

#include <stdexcept>

namespace zSpace
{
	ZSPACE_INLINE zItPointCloudVertex::zItPointCloudVertex()
		: index(0), pointsObj(nullptr)
	{
	}

	ZSPACE_INLINE zItPointCloudVertex::zItPointCloudVertex(zObjectPointCloud& _pointsObj)
		: index(0), pointsObj(&_pointsObj)
	{
	}

	ZSPACE_INLINE zItPointCloudVertex::zItPointCloudVertex(zObjectPointCloud& _pointsObj, int _index)
		: index(_index), pointsObj(&_pointsObj)
	{
		if (_index < 0 || _index >= size())
			throw std::invalid_argument("error: point-cloud vertex index out of bounds");
	}

	ZSPACE_INLINE void zItPointCloudVertex::begin() { index = 0; }
	ZSPACE_INLINE void zItPointCloudVertex::operator++(int) { ++index; }
	ZSPACE_INLINE void zItPointCloudVertex::operator--(int) { --index; }
	ZSPACE_INLINE bool zItPointCloudVertex::end() { return !pointsObj || index >= size(); }
	ZSPACE_INLINE void zItPointCloudVertex::reset() { begin(); }

	ZSPACE_INLINE int zItPointCloudVertex::size()
	{
		return pointsObj
			? static_cast<int>(pointsObj->impl->storage.vertices.size())
			: 0;
	}

	ZSPACE_INLINE void zItPointCloudVertex::deactivate()
	{
		pointsObj->impl->storage.vertices[index].reset();
	}

	ZSPACE_INLINE int zItPointCloudVertex::getId()
	{
		return pointsObj->impl->storage.vertices[index].getId();
	}

	ZSPACE_INLINE zPoint zItPointCloudVertex::getPosition()
	{
		return pointsObj->impl->storage.vertexPositions[getId()];
	}

	ZSPACE_INLINE zPoint* zItPointCloudVertex::getRawPosition()
	{
		return &pointsObj->impl->storage.vertexPositions[getId()];
	}

	ZSPACE_INLINE zColor zItPointCloudVertex::getColor()
	{
		return pointsObj->impl->storage.vertexColors[getId()];
	}

	ZSPACE_INLINE zColor* zItPointCloudVertex::getRawColor()
	{
		return &pointsObj->impl->storage.vertexColors[getId()];
	}

	ZSPACE_INLINE void zItPointCloudVertex::setId(int _id)
	{
		pointsObj->impl->storage.vertices[index].setId(_id);
	}

	ZSPACE_INLINE void zItPointCloudVertex::setPosition(zPoint& pos)
	{
		pointsObj->impl->storage.vertexPositions[getId()] = pos;
	}

	ZSPACE_INLINE void zItPointCloudVertex::setColor(zColor col)
	{
		pointsObj->impl->storage.vertexColors[getId()] = col;
	}

	ZSPACE_INLINE bool zItPointCloudVertex::isActive()
	{
		return pointsObj->impl->storage.vertices[index].isActive();
	}

	ZSPACE_INLINE bool zItPointCloudVertex::operator==(zItPointCloudVertex& other)
	{
		return pointsObj == other.pointsObj && index == other.index;
	}

	ZSPACE_INLINE bool zItPointCloudVertex::operator!=(zItPointCloudVertex& other)
	{
		return !(*this == other);
	}
}
