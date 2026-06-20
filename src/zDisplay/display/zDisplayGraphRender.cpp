#include <zspace/zDisplay/display/zDisplayGraph.h>
#include <src/zDisplay/display/internal/zDisplaySceneImpl.h>

#include <zspace/zInterface/functionsets/zFnGraph.h>
#include <zspace/zInterface/iterators/zItGraph.h>
#include <zspace/zInterface/objects/zObjectGraph.h>

namespace zSpace
{
	ZSPACE_INLINE void zDisplayGraph::draw(zObjectGraph& object, zDisplayScene& scene) const
	{
		zUtilsDisplay& displayUtils = scene.impl_->backend;
		if (objectVisible()) drawElements(object, scene);
		if (transformVisible()) displayUtils.drawTransform(object.transformationMatrix);
	}

	ZSPACE_INLINE void zDisplayGraph::appendToBuffer(
		zObjectGraph& object,
		zDisplayScene& scene) const
	{
		zUtilsDisplay& displayUtils = scene.impl_->backend;
		zDisplayBufferOffsets& offsets = scene.impl_->graphBuffers[&object];
		const int baseVertex = displayUtils.bufferObj.nVertices;

		zIntArray edgeIndices;
		for (zItGraphEdge edge(object); !edge.end(); edge++)
		{
			if (!edge.isActive()) continue;
			zIntArray vertices;
			edge.getVertices(vertices);
			for (int vertexId : vertices) edgeIndices.push_back(vertexId + baseVertex);
		}
		offsets.edge = displayUtils.bufferObj.appendEdgeIndices(edgeIndices);

		zFnGraph fnGraph(object);
		zPointArray positions;
		zColorArray colors;
		fnGraph.getVertexPositions(positions);
		fnGraph.getVertexColors(colors);

		if (!positions.empty())
		{
			offsets.vertex = displayUtils.bufferObj.appendVertexAttributes(
				positions.data(),
				nullptr,
				static_cast<int>(positions.size()));
		}
		if (!colors.empty())
		{
			offsets.vertexColor = displayUtils.bufferObj.appendVertexColors(
				colors.data(),
				static_cast<int>(colors.size()));
		}
	}

	ZSPACE_INLINE void zDisplayGraph::drawElements(
		zObjectGraph& object,
		zDisplayScene& scene) const
	{
		zUtilsDisplay& displayUtils = scene.impl_->backend;
		zFnGraph fnGraph(object);

		zPointArray positions;
		zColorArray vertexColors;
		zDoubleArray vertexWeights;
		zColorArray edgeColors;
		zDoubleArray edgeWeights;
		fnGraph.getVertexPositions(positions);
		fnGraph.getVertexColors(vertexColors);
		fnGraph.getVertexWeights(vertexWeights);
		fnGraph.getEdgeColors(edgeColors);
		fnGraph.getEdgeWeights(edgeWeights);

		if (verticesVisible())
		{
			for (zItGraphVertex vertex(object); !vertex.end(); vertex++)
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
			for (zItGraphEdge edge(object); !edge.end(); edge++)
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
			if (edgeCenters().size() != static_cast<size_t>(fnGraph.numEdges()))
				throw std::invalid_argument("error: edge centers are not computed.");

			zPointArray centers = edgeCenters();
			zColor color(0, 0.8, 0, 1);
			displayUtils.drawEdgeIds(fnGraph.numEdges(), centers.data(), color);
		}
	}
}
