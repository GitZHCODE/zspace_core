#include <src/zCore/geometry/detail/zMeshFaceListStorage.h>

#include <algorithm>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace zSpace::detail
{
	void zMeshFaceListStorage::clear()
	{
		positions.clear();
		faceOffsets.clear();
		faceVertexIndices.clear();
		edgeVertexIndices.clear();
		edgeFirstVertices.clear();
		edgeUseCounts.clear();
		vertexNormals.clear();
		faceNormals.clear();
		vertexColors.clear();
		edgeColors.clear();
		faceColors.clear();
		vertexWeights.clear();
		edgeWeights.clear();
	}

	void zMeshFaceListStorage::set(const zPointArray& newPositions,
		const zIntArray& polygonCounts, const zIntArray& polygonConnects)
	{
		clear();
		positions = newPositions;
		faceVertexIndices = polygonConnects;
		initializeFaces(polygonCounts);
	}

	void zMeshFaceListStorage::setMoved(zPointArray&& newPositions,
		const zIntArray& polygonCounts, zIntArray&& polygonConnects)
	{
		clear();
		positions = std::move(newPositions);
		faceVertexIndices = std::move(polygonConnects);
		initializeFaces(polygonCounts);
	}

	void zMeshFaceListStorage::initializeFaces(const zIntArray& polygonCounts)
	{
		faceOffsets.reserve(polygonCounts.size() + 1);
		faceOffsets.push_back(0);
		for (int count : polygonCounts)
			faceOffsets.push_back(faceOffsets.back() + count);

		std::string error;
		if (!validate(&error))
		{
			clear();
			throw std::invalid_argument(error);
		}

		vertexNormals.assign(positions.size(), zVector());
		faceNormals.assign(polygonCounts.size(), zVector());
		vertexColors.assign(positions.size(), zColor(1, 0, 0, 1));
		faceColors.assign(polygonCounts.size(), zColor(0.5, 0.5, 0.5, 1));
		vertexWeights.assign(positions.size(), 2.0);
		rebuildEdges();
	}

	void zMeshFaceListStorage::polygonData(zIntArray& polygonConnects,
		zIntArray& polygonCounts) const
	{
		polygonConnects = faceVertexIndices;
		polygonCounts.clear();
		polygonCounts.reserve(numFaces());
		for (std::size_t i = 1; i < faceOffsets.size(); ++i)
			polygonCounts.push_back(faceOffsets[i] - faceOffsets[i - 1]);
	}

	void zMeshFaceListStorage::rebuildEdges()
	{
		auto keyOf = [](int a, int b) {
			return (std::uint64_t(std::uint32_t(a)) << 32) | std::uint32_t(b);
		};
		const bool hasPreviousEdgeAttributes = !edgeVertexIndices.empty() && (!edgeColors.empty() || !edgeWeights.empty());
		std::unordered_map<std::uint64_t, int> oldEdges;
		zColorArray previousColors;
		zDoubleArray previousWeights;
		if (hasPreviousEdgeAttributes)
		{
			oldEdges.reserve(numEdges());
			for (std::size_t i = 0; i + 1 < edgeVertexIndices.size(); i += 2)
			{
				const auto endpoints = std::minmax(edgeVertexIndices[i], edgeVertexIndices[i + 1]);
				oldEdges[keyOf(endpoints.first, endpoints.second)] = static_cast<int>(i / 2);
			}
			previousColors = std::move(edgeColors);
			previousWeights = std::move(edgeWeights);
		}

		const std::size_t estimatedEdges = (faceVertexIndices.size() / 2) + 1;
		edgeVertexIndices.clear();
		edgeFirstVertices.clear();
		edgeUseCounts.clear();
		edgeColors.clear();
		edgeWeights.clear();
		edgeVertexIndices.reserve(estimatedEdges * 2);
		edgeFirstVertices.reserve(estimatedEdges);
		edgeUseCounts.reserve(estimatedEdges);
		edgeColors.reserve(estimatedEdges);
		edgeWeights.reserve(estimatedEdges);

		std::vector<int> head(positions.size(), -1);
		std::vector<int> edgeTarget;
		std::vector<int> edgeNext;
		edgeTarget.reserve(estimatedEdges);
		edgeNext.reserve(estimatedEdges);

		for (int faceId = 0; faceId < numFaces(); ++faceId)
		{
			const int begin = faceOffsets[faceId];
			const int end = faceOffsets[faceId + 1];
			for (int i = begin; i < end; ++i)
			{
				const int next = (i + 1 < end) ? i + 1 : begin;
				const auto endpoints = std::minmax(faceVertexIndices[i], faceVertexIndices[next]);
				const int u = endpoints.first;
				const int v = endpoints.second;

				int existingEdge = -1;
				for (int e = head[u]; e != -1; e = edgeNext[e])
				{
					if (edgeTarget[e] == v)
					{
						existingEdge = e;
						break;
					}
				}
				if (existingEdge != -1)
				{
					edgeUseCounts[existingEdge]++;
					continue;
				}

				const int newEdgeId = static_cast<int>(edgeTarget.size());
				edgeTarget.push_back(v);
				edgeNext.push_back(head[u]);
				head[u] = newEdgeId;

				edgeVertexIndices.push_back(u);
				edgeVertexIndices.push_back(v);
				edgeFirstVertices.push_back(faceVertexIndices[i]);
				edgeUseCounts.push_back(1);
				if (hasPreviousEdgeAttributes)
				{
					const auto key = keyOf(u, v);
					const auto old = oldEdges.find(key);
					edgeColors.push_back(old != oldEdges.end() && old->second < static_cast<int>(previousColors.size())
						? previousColors[old->second] : zColor(0, 0, 0, 0));
					edgeWeights.push_back(old != oldEdges.end() && old->second < static_cast<int>(previousWeights.size())
						? previousWeights[old->second] : 1.0);
				}
				else
				{
					edgeColors.push_back(zColor(0, 0, 0, 0));
					edgeWeights.push_back(1.0);
				}
			}
		}
	}

	bool zMeshFaceListStorage::validate(std::string* error) const
	{
		auto fail = [&](const std::string& message)
		{
			if (error) *error = message;
			return false;
		};

		if (faceOffsets.empty())
		{
			if (faceVertexIndices.empty()) return true;
			return fail("Face offsets are missing.");
		}
		if (faceOffsets.front() != 0) return fail("Face offsets must start at zero.");
		if (faceOffsets.back() != static_cast<int>(faceVertexIndices.size()))
			return fail("Face offsets do not match the face-index array.");

		for (std::size_t i = 1; i < faceOffsets.size(); ++i)
		{
			if (faceOffsets[i] < faceOffsets[i - 1]) return fail("Face offsets are not ordered.");
			if (faceOffsets[i] - faceOffsets[i - 1] < 3)
				return fail("Every polygon must contain at least three vertices.");
		}
		for (int vertex : faceVertexIndices)
		{
			if (vertex < 0 || vertex >= static_cast<int>(positions.size()))
				return fail("A polygon references an invalid vertex index.");
		}
		return true;
	}
}
