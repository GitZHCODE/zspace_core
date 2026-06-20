#include <src/zCore/geometry/detail/zMeshFaceListStorage.h>

#include <algorithm>
#include <map>
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
		std::map<std::pair<int, int>, int> oldEdges;
		for (std::size_t i = 0; i + 1 < edgeVertexIndices.size(); i += 2)
		{
			const auto endpoints = std::minmax(edgeVertexIndices[i], edgeVertexIndices[i + 1]);
			oldEdges[{ endpoints.first, endpoints.second }] = static_cast<int>(i / 2);
		}

		const zColorArray previousColors = edgeColors;
		const zDoubleArray previousWeights = edgeWeights;
		edgeVertexIndices.clear();
		edgeColors.clear();
		edgeWeights.clear();

		std::map<std::pair<int, int>, int> edges;
		for (int faceId = 0; faceId < numFaces(); ++faceId)
		{
			const int begin = faceOffsets[faceId];
			const int end = faceOffsets[faceId + 1];
			for (int i = begin; i < end; ++i)
			{
				const int next = (i + 1 < end) ? i + 1 : begin;
				const auto endpoints = std::minmax(faceVertexIndices[i], faceVertexIndices[next]);
				const std::pair<int, int> key{ endpoints.first, endpoints.second };
				if (edges.find(key) != edges.end()) continue;

				edges[key] = static_cast<int>(edges.size());
				edgeVertexIndices.push_back(key.first);
				edgeVertexIndices.push_back(key.second);
				const auto old = oldEdges.find(key);
				edgeColors.push_back(old != oldEdges.end() && old->second < static_cast<int>(previousColors.size())
					? previousColors[old->second] : zColor(0, 0, 0, 0));
				edgeWeights.push_back(old != oldEdges.end() && old->second < static_cast<int>(previousWeights.size())
					? previousWeights[old->second] : 1.0);
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
