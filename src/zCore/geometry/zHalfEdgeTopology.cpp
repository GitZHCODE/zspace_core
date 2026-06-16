// This file is part of zspace, a simple C++ collection of geometry data-structures & algorithms,
// data analysis & visualization framework.

#include <src/zCore/geometry/detail/zHalfEdgeTopology.h>

namespace zSpace
{
	namespace detail
	{
		ZSPACE_INLINE zHalfEdgeTopology::zHalfEdgeTopology()
		{
			n_v = n_e = n_he = 0;
		}

		ZSPACE_INLINE zHalfEdgeTopology::~zHalfEdgeTopology() {}

		ZSPACE_INLINE void zHalfEdgeTopology::setNumVertices(int _n_v, bool setMax)
		{
			n_v = _n_v;
		}

		ZSPACE_INLINE void zHalfEdgeTopology::setNumEdges(int _n_e, bool setMax)
		{
			n_e = _n_e;
			n_he = _n_e * 2;
		}

		ZSPACE_INLINE bool zHalfEdgeTopology::vertexExists(zPoint pos, int& outVertexId, int precisionfactor)
		{
			double factor = pow(10, precisionfactor);
			double x = std::round(pos.x * factor) / factor;
			double y = std::round(pos.y * factor) / factor;
			double z = std::round(pos.z * factor) / factor;

			string hashKey = (to_string(x) + "," + to_string(y) + "," + to_string(z));
			auto existing = positionVertex.find(hashKey);

			if (existing != positionVertex.end())
			{
				outVertexId = existing->second;
				return true;
			}

			outVertexId = -1;
			return false;
		}

		ZSPACE_INLINE void zHalfEdgeTopology::addToPositionMap(zPoint& pos, int index, int precisionfactor)
		{
			double factor = pow(10, precisionfactor);
			double x = std::round(pos.x * factor) / factor;
			double y = std::round(pos.y * factor) / factor;
			double z = std::round(pos.z * factor) / factor;

			string hashKey = (to_string(x) + "," + to_string(y) + "," + to_string(z));
			positionVertex[hashKey] = index;
		}

		ZSPACE_INLINE void zHalfEdgeTopology::removeFromPositionMap(zPoint& pos, int precisionfactor)
		{
			double factor = pow(10, precisionfactor);
			double x = std::round(pos.x * factor) / factor;
			double y = std::round(pos.y * factor) / factor;
			double z = std::round(pos.z * factor) / factor;

			string removeHashKey = (to_string(x) + "," + to_string(y) + "," + to_string(z));
			positionVertex.erase(removeHashKey);
		}

		ZSPACE_INLINE void zHalfEdgeTopology::addToHalfEdgesMap(int v1, int v2, int index)
		{
			string e1 = (to_string(v1) + "," + to_string(v2));
			existingHalfEdges[e1] = index;

			string e2 = (to_string(v2) + "," + to_string(v1));
			existingHalfEdges[e2] = index + 1;
		}

		ZSPACE_INLINE void zHalfEdgeTopology::removeFromHalfEdgesMap(int v1, int v2)
		{
			string e1 = (to_string(v1) + "," + to_string(v2));
			existingHalfEdges.erase(e1);

			string e2 = (to_string(v2) + "," + to_string(v1));
			existingHalfEdges.erase(e2);
		}

		ZSPACE_INLINE bool zHalfEdgeTopology::halfEdgeExists(int v1, int v2, int& outEdgeId)
		{
			string e1 = (to_string(v1) + "," + to_string(v2));
			auto existing = existingHalfEdges.find(e1);

			if (existing != existingHalfEdges.end())
			{
				outEdgeId = existing->second;
				return true;
			}

			outEdgeId = -1;
			return false;
		}
	}
}
