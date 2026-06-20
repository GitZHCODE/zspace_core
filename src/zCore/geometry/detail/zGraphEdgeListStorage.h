#ifndef ZSPACE_DETAIL_GRAPH_EDGE_LIST_STORAGE_H
#define ZSPACE_DETAIL_GRAPH_EDGE_LIST_STORAGE_H

#pragma once

#include <zspace/zCore/base/zTypeDef.h>

#include <string>

namespace zSpace::detail
{
	class ZSPACE_CORE zGraphEdgeListStorage
	{
	public:
		zPointArray positions;
		zIntArray edgeVertexIndices;

		zColorArray vertexColors;
		zColorArray edgeColors;
		zDoubleArray vertexWeights;
		zDoubleArray edgeWeights;

		void clear();
		void set(const zPointArray& positions, const zIntArray& edgeConnects);
		bool addVertex(const zPoint& position);
		bool addEdge(int v0, int v1, bool checkDuplicates = false, int* outEdgeId = nullptr);
		bool vertexExists(const zPoint& position, int& outVertexId, int precisionfactor = PRECISION) const;
		bool edgeExists(int v0, int v1, int& outEdgeId) const;
		bool validate(std::string* error = nullptr) const;

		int numVertices() const { return static_cast<int>(positions.size()); }
		int numEdges() const { return static_cast<int>(edgeVertexIndices.size() / 2); }
		int numHalfEdges() const { return static_cast<int>(edgeVertexIndices.size()); }
	};
}

#endif
