#include <src/zCore/geometry/detail/zGraphEdgeListStorage.h>

#include <zspace/zCore/utilities/zUtilsCore.h>

#include <cmath>

namespace zSpace::detail
{
	ZSPACE_INLINE void zGraphEdgeListStorage::clear()
	{
		positions.clear();
		edgeVertexIndices.clear();
		vertexColors.clear();
		edgeColors.clear();
		vertexWeights.clear();
		edgeWeights.clear();
	}

	ZSPACE_INLINE void zGraphEdgeListStorage::set(const zPointArray& _positions, const zIntArray& edgeConnects)
	{
		positions = _positions;
		edgeVertexIndices = edgeConnects;

		if (vertexColors.size() != positions.size())
			vertexColors.assign(positions.size(), zColor(1, 0, 0, 1));
		if (vertexWeights.size() != positions.size())
			vertexWeights.assign(positions.size(), 1.0);
		if (edgeColors.size() != numEdges())
			edgeColors.assign(numEdges(), zColor(0, 0, 0, 1));
		if (edgeWeights.size() != numEdges())
			edgeWeights.assign(numEdges(), 2.0);
	}

	ZSPACE_INLINE bool zGraphEdgeListStorage::addVertex(const zPoint& position)
	{
		positions.push_back(position);
		vertexColors.push_back(zColor(1, 0, 0, 1));
		vertexWeights.push_back(1.0);
		return false;
	}

	ZSPACE_INLINE bool zGraphEdgeListStorage::addEdge(int v0, int v1, bool checkDuplicates, int* outEdgeId)
	{
		if (checkDuplicates)
		{
			int existing = -1;
			if (edgeExists(v0, v1, existing))
			{
				if (outEdgeId) *outEdgeId = existing;
				return false;
			}
		}

		if (outEdgeId) *outEdgeId = numEdges();
		edgeVertexIndices.push_back(v0);
		edgeVertexIndices.push_back(v1);
		edgeColors.push_back(zColor(0, 0, 0, 1));
		edgeWeights.push_back(2.0);
		return false;
	}

	ZSPACE_INLINE bool zGraphEdgeListStorage::vertexExists(const zPoint& position, int& outVertexId, int precisionfactor) const
	{
		zUtilsCore coreUtils;
		zVector target = position;
		target = coreUtils.factoriseVector(target, precisionfactor);
		for (int i = 0; i < static_cast<int>(positions.size()); ++i)
		{
			zVector current = positions[i];
			current = coreUtils.factoriseVector(current, precisionfactor);
			if (target.distanceTo(current) < std::pow(10.0, -1 * precisionfactor))
			{
				outVertexId = i;
				return true;
			}
		}

		outVertexId = -1;
		return false;
	}

	ZSPACE_INLINE bool zGraphEdgeListStorage::edgeExists(int v0, int v1, int& outEdgeId) const
	{
		for (int i = 0; i + 1 < static_cast<int>(edgeVertexIndices.size()); i += 2)
		{
			const int a = edgeVertexIndices[i];
			const int b = edgeVertexIndices[i + 1];
			if ((a == v0 && b == v1) || (a == v1 && b == v0))
			{
				outEdgeId = i / 2;
				return true;
			}
		}

		outEdgeId = -1;
		return false;
	}

	ZSPACE_INLINE bool zGraphEdgeListStorage::validate(std::string* error) const
	{
		if (edgeVertexIndices.size() % 2 != 0)
		{
			if (error) *error = "Graph edge connectivity must contain pairs of vertex indices.";
			return false;
		}

		for (int i = 0; i < static_cast<int>(edgeVertexIndices.size()); ++i)
		{
			const int vertexId = edgeVertexIndices[i];
			if (vertexId < 0 || vertexId >= static_cast<int>(positions.size()))
			{
				if (error) *error = "Graph edge connectivity contains an out-of-range vertex index.";
				return false;
			}
		}

		return true;
	}
}
