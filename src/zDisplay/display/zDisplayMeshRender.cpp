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

		zIntArray edgeIndices;
		for (zItMeshEdge edge(object); !edge.end(); edge++)
		{
			if (!edge.isActive()) continue;
			const int edgeId = edge.getId();
			if (dihedralEdgesOnly)
			{
				if (edgeId >= static_cast<int>(dihedralAngles.size())) continue;
				const double angle = dihedralAngles[edgeId];
				if (std::abs(angle) <= angleThreshold && angle != -1) continue;
			}

			zIntArray vertices;
			edge.getVertices(vertices);
			for (int vertexId : vertices) edgeIndices.push_back(vertexId + baseVertex);
		}
		offsets.edge = displayUtils.bufferObj.appendEdgeIndices(edgeIndices);

		zIntArray faceIndices;
		for (zItMeshFace face(object); !face.end(); face++)
		{
			if (!face.isActive()) continue;
			zIntArray vertices;
			face.getVertices(vertices);
			for (int vertexId : vertices) faceIndices.push_back(vertexId + baseVertex);
		}
		offsets.face = displayUtils.bufferObj.appendFaceIndices(faceIndices);

		zFnMesh fnMesh(object);
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
			for (zItMeshVertex vertex(object); !vertex.end(); vertex++)
			{
				if (!vertex.isActive()) continue;
				const int id = vertex.getId();
				zPoint position = vertex.getPosition();
				const zColor color = id < static_cast<int>(vertexColors.size())
					? vertexColors[id]
					: zColor(1, 0, 0, 1);
				const double weight = id < static_cast<int>(vertexWeights.size())
					? vertexWeights[id]
					: 1.0;
				displayUtils.drawPoint(position, color, weight);
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
			for (zItMeshEdge edge(object); !edge.end(); edge++)
			{
				if (!edge.isActive()) continue;
				zPointArray edgePositions;
				edge.getVertexPositions(edgePositions);
				if (edgePositions.size() < 2) continue;

				const int id = edge.getId();
				const zColor color = id < static_cast<int>(edgeColors.size())
					? edgeColors[id]
					: zColor();
				const double weight = id < static_cast<int>(edgeWeights.size())
					? edgeWeights[id]
					: 1.0;
				displayUtils.drawLine(edgePositions[0], edgePositions[1], color, weight);
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
			for (zItMeshFace face(object); !face.end(); face++)
			{
				if (!face.isActive()) continue;
				zPointArray facePositions;
				face.getVertexPositions(facePositions);
				if (facePositions.empty()) continue;

				const int id = face.getId();
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

		for (zItMeshEdge edge(object); !edge.end(); edge++)
		{
			if (!edge.isActive()) continue;
			const int id = edge.getId();
			if (std::abs(dihedralAngles()[id]) <= dihedralAngleThreshold()) continue;

			zPointArray positions;
			edge.getVertexPositions(positions);
			if (positions.size() < 2) continue;

			const zColor color = id < static_cast<int>(colors.size()) ? colors[id] : zColor();
			const double weight = id < static_cast<int>(weights.size()) ? weights[id] : 1.0;
			scene.impl_->backend.drawLine(positions[0], positions[1], color, weight);
		}
	}

	ZSPACE_INLINE void zDisplayMesh::drawVertexNormals(
		zObjectMesh& object,
		zDisplayScene& scene) const
	{
		for (zItMeshVertex vertex(object); !vertex.end(); vertex++)
		{
			if (!vertex.isActive()) continue;
			zPoint start = vertex.getPosition();
			zPoint end = start + vertex.getNormal() * normalScale();
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

		for (zItMeshFace face(object); !face.end(); face++)
		{
			if (!face.isActive()) continue;
			const int id = face.getId();
			zPoint start = faceCenters()[id];
			zPoint end = start + face.getNormal() * normalScale();
			scene.impl_->backend.drawLine(start, end, zColor(0, 1, 0, 1));
		}
	}
}
