#ifndef ZSPACE_DETAIL_MESH_FACE_LIST_STORAGE_H
#define ZSPACE_DETAIL_MESH_FACE_LIST_STORAGE_H

#pragma once

#include <zspace/zCore/base/zTypeDef.h>

#include <string>

namespace zSpace::detail
{
	class ZSPACE_CORE zMeshFaceListStorage
	{
	public:
		zPointArray positions;
		zIntArray faceOffsets;
		zIntArray faceVertexIndices;
		zIntArray edgeVertexIndices;

		zVectorArray vertexNormals;
		zVectorArray faceNormals;
		zColorArray vertexColors;
		zColorArray edgeColors;
		zColorArray faceColors;
		zDoubleArray vertexWeights;
		zDoubleArray edgeWeights;

		void clear();
		void set(const zPointArray& positions, const zIntArray& polygonCounts,
			const zIntArray& polygonConnects);
		void polygonData(zIntArray& polygonConnects, zIntArray& polygonCounts) const;
		void rebuildEdges();
		bool validate(std::string* error = nullptr) const;

		int numVertices() const { return static_cast<int>(positions.size()); }
		int numEdges() const { return static_cast<int>(edgeVertexIndices.size() / 2); }
		int numFaces() const { return faceOffsets.empty() ? 0 : static_cast<int>(faceOffsets.size() - 1); }
	};
}

#endif
