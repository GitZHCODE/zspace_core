#include <zspace/zDisplay/display/zDisplayMesh.h>
#include <src/zDisplay/display/internal/zDisplaySceneImpl.h>

#include <zspace/zInterface/functionsets/zFnMesh.h>
#include <zspace/zInterface/iterators/zItMesh.h>
#include <zspace/zInterface/objects/zObjMesh.h>

namespace zSpace
{
	ZSPACE_INLINE void zDisplayMesh::draw(zObjectMesh& object, zDisplayScene& scene) const
	{
		zUtilsDisplay& displayUtils = scene.impl_->backend;
		if (objectVisible())
		{
			drawElements(object, scene);
			if (dihedralEdgesVisible()) drawDihedralEdges(object, scene);
			if (vertexNormalsVisible()) drawVertexNormals(object, scene);
			if (faceNormalsVisible()) drawFaceNormals(object, scene);
		}

		if (transformVisible()) displayUtils.drawTransform(object.transformationMatrix);
	}

	ZSPACE_INLINE void zDisplayMesh::appendToBuffer(
		zObjectMesh& object,
		zDisplayScene& scene,
		const zDoubleArray& dihedralAngles,
		bool dihedralEdgesOnly,
		double angleThreshold) const
	{
		zUtilsDisplay& displayUtils = scene.impl_->backend;
		zDisplayBufferOffsets& offsets = scene.impl_->meshBuffers[&object];
		const int baseVertex = displayUtils.bufferObj.nVertices;

		zFnMesh fnMesh(object);
		zIntArray edgeConnects;
		fnMesh.getEdgeData(edgeConnects);
		zIntArray edgeIndices;
		for (int edgeId = 0; edgeId < static_cast<int>(edgeConnects.size() / 2); ++edgeId)
		{
			if (dihedralEdgesOnly)
			{
				if (edgeId >= static_cast<int>(dihedralAngles.size())) continue;
				const double angle = dihedralAngles[edgeId];
				if (std::abs(angle) <= angleThreshold && angle != -1) continue;
			}

			edgeIndices.push_back(edgeConnects[edgeId * 2] + baseVertex);
			edgeIndices.push_back(edgeConnects[edgeId * 2 + 1] + baseVertex);
		}
		offsets.edge = displayUtils.bufferObj.appendEdgeIndices(edgeIndices);

		zIntArray faceIndices;
		zIntArray faceCounts;
		fnMesh.getPolygonData(faceIndices, faceCounts);
		for (int& vertexId : faceIndices) vertexId += baseVertex;
		offsets.face = displayUtils.bufferObj.appendFaceIndices(faceIndices);

		zPointArray positions;
		zVectorArray normals;
		zColorArray colors;
		fnMesh.getVertexPositions(positions);
		fnMesh.getVertexNormals(normals);
		fnMesh.getVertexColors(colors);

		if (!positions.empty())
		{
			zVector* normalData = normals.size() == positions.size() ? normals.data() : nullptr;
			offsets.vertex = displayUtils.bufferObj.appendVertexAttributes(
				positions.data(),
				normalData,
				static_cast<int>(positions.size()));
		}
		if (!colors.empty())
		{
			offsets.vertexColor = displayUtils.bufferObj.appendVertexColors(
				colors.data(),
				static_cast<int>(colors.size()));
		}
	}

	ZSPACE_INLINE void zDisplayMesh::drawElements(
		zObjectMesh& object,
		zDisplayScene& scene) const
	{
		zUtilsDisplay& displayUtils = scene.impl_->backend;
		zFnMesh fnMesh(object);

		zPointArray positions;
		zColorArray vertexColors;
		zDoubleArray vertexWeights;
		zColorArray edgeColors;
		zDoubleArray edgeWeights;
		zColorArray faceColors;
		fnMesh.getVertexPositions(positions);
		fnMesh.getVertexColors(vertexColors);
		fnMesh.getVertexWeights(vertexWeights);
		fnMesh.getEdgeColors(edgeColors);
		fnMesh.getEdgeWeights(edgeWeights);
		fnMesh.getFaceColors(faceColors);

		if (verticesVisible())
		{
			for (int id = 0; id < static_cast<int>(positions.size()); ++id)
			{
				const zColor color = id < static_cast<int>(vertexColors.size())
					? vertexColors[id]
					: zColor(1, 0, 0, 1);
				const double weight = id < static_cast<int>(vertexWeights.size())
					? vertexWeights[id]
					: 1.0;
				displayUtils.drawPoint(positions[id], color, weight);
			}
		}

		if (vertexIdsVisible() && !positions.empty())
		{
			zColor color(0.8, 0, 0, 1);
			displayUtils.drawVertexIds(
				static_cast<int>(positions.size()),
				positions.data(),
				color);
		}

		if (edgesVisible())
		{
			zIntArray edgeConnects;
			fnMesh.getEdgeData(edgeConnects);
			for (int id = 0; id < static_cast<int>(edgeConnects.size() / 2); ++id)
			{
				const zColor color = id < static_cast<int>(edgeColors.size())
					? edgeColors[id]
					: zColor();
				const double weight = id < static_cast<int>(edgeWeights.size())
					? edgeWeights[id]
					: 1.0;
				const int a = edgeConnects[id * 2];
				const int b = edgeConnects[id * 2 + 1];
				displayUtils.drawLine(positions[a], positions[b], color, weight);
			}
		}

		if (edgeIdsVisible())
		{
			if (edgeCenters().size() != static_cast<size_t>(fnMesh.numEdges()))
				throw std::invalid_argument("error: edge centers are not computed.");

			zPointArray centers = edgeCenters();
			zColor color(0, 0.8, 0, 1);
			displayUtils.drawEdgeIds(fnMesh.numEdges(), centers.data(), color);
		}

		if (facesVisible())
		{
			zIntArray polygonConnects;
			zIntArray polygonCounts;
			fnMesh.getPolygonData(polygonConnects, polygonCounts);
			int cursor = 0;
			for (int id = 0; id < static_cast<int>(polygonCounts.size()); ++id)
			{
				zPointArray facePositions;
				for (int i = 0; i < polygonCounts[id]; ++i)
					facePositions.push_back(positions[polygonConnects[cursor++]]);
				const zColor color = id < static_cast<int>(faceColors.size())
					? faceColors[id]
					: zColor(0.5, 0.5, 0.5, 1);
				displayUtils.drawPolygon(facePositions, color);
			}
		}

		if (faceIdsVisible())
		{
			if (faceCenters().size() != static_cast<size_t>(fnMesh.numPolygons()))
				throw std::invalid_argument("error: face centers are not computed.");

			zPointArray centers = faceCenters();
			zColor color(0, 0, 0.8, 1);
			displayUtils.drawFaceIds(fnMesh.numPolygons(), centers.data(), color);
		}
	}

	ZSPACE_INLINE void zDisplayMesh::drawDihedralEdges(
		zObjectMesh& object,
		zDisplayScene& scene) const
	{
		zFnMesh fnMesh(object);
		if (dihedralAngles().size() != static_cast<size_t>(fnMesh.numEdges()))
			throw std::invalid_argument("error: dihedral angles are not computed.");

		zColorArray colors;
		zDoubleArray weights;
		fnMesh.getEdgeColors(colors);
		fnMesh.getEdgeWeights(weights);

		zPointArray vertexPositions;
		zIntArray edgeConnects;
		fnMesh.getVertexPositions(vertexPositions);
		fnMesh.getEdgeData(edgeConnects);
		for (int id = 0; id < static_cast<int>(edgeConnects.size() / 2); ++id)
		{
			if (std::abs(dihedralAngles()[id]) <= dihedralAngleThreshold()) continue;

			const zColor color = id < static_cast<int>(colors.size()) ? colors[id] : zColor();
			const double weight = id < static_cast<int>(weights.size()) ? weights[id] : 1.0;
			scene.impl_->backend.drawLine(
				vertexPositions[edgeConnects[id * 2]],
				vertexPositions[edgeConnects[id * 2 + 1]], color, weight);
		}
	}

	ZSPACE_INLINE void zDisplayMesh::drawVertexNormals(
		zObjectMesh& object,
		zDisplayScene& scene) const
	{
		zFnMesh fnMesh(object);
		zPointArray positions;
		zVectorArray normals;
		fnMesh.getVertexPositions(positions);
		fnMesh.getVertexNormals(normals);
		for (int id = 0; id < static_cast<int>(positions.size()) && id < static_cast<int>(normals.size()); ++id)
		{
			zPoint start = positions[id];
			zPoint end = start + normals[id] * normalScale();
			scene.impl_->backend.drawLine(start, end, zColor(0, 1, 0, 1));
		}
	}

	ZSPACE_INLINE void zDisplayMesh::drawFaceNormals(
		zObjectMesh& object,
		zDisplayScene& scene) const
	{
		zFnMesh fnMesh(object);
		if (faceCenters().size() != static_cast<size_t>(fnMesh.numPolygons()))
			throw std::invalid_argument("error: face centers do not match the mesh faces.");

		zVectorArray normals;
		fnMesh.getFaceNormals(normals);
		for (int id = 0; id < static_cast<int>(normals.size()); ++id)
		{
			zPoint start = faceCenters()[id];
			zPoint end = start + normals[id] * normalScale();
			scene.impl_->backend.drawLine(start, end, zColor(0, 1, 0, 1));
		}
	}
}
