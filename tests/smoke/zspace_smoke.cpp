#include <zspace/interface.h>

#include <iostream>
#include <stdexcept>

namespace
{
	using namespace zSpace;

	void require(bool condition, const char* message)
	{
		if (!condition) throw std::runtime_error(message);
	}

	void testMesh(zObjectMesh& mesh)
	{
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(1, 1, 0),
			zPoint(0, 1, 0)
		};
		zIntArray polygonCounts = { 4 };
		zIntArray polygonConnects = { 0, 1, 2, 3 };

		zFnMesh fnMesh(mesh);
		fnMesh.create(positions, polygonCounts, polygonConnects);

		require(fnMesh.numVertices() == 4, "mesh vertex count");
		require(fnMesh.numEdges() == 4, "mesh edge count");
		require(fnMesh.numPolygons() == 1, "mesh polygon count");

		zItMeshVertex vertex(mesh, 0);
		vertex.setPosition(zPoint(-0.25, 0, 0));
		require(vertex.getPosition().x == -0.25, "mesh vertex edit");
	}

	void testGraph(zObjectGraph& graph)
	{
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(1, 1, 0)
		};
		zIntArray edgeConnects = { 0, 1, 1, 2 };

		zFnGraph fnGraph(graph);
		fnGraph.create(positions, edgeConnects);

		require(fnGraph.numVertices() == 3, "graph vertex count");
		require(fnGraph.numEdges() == 2, "graph edge count");

		zItGraphVertex vertex(graph, 2);
		zPoint updated(1, 1.5, 0);
		vertex.setPosition(updated);
		require(vertex.getPosition().y == 1.5, "graph vertex edit");
	}

	void testMeshToGraph(zObjectMesh& mesh)
	{
		zObjectGraph graph;
		zFnGraph fnGraph(graph);
		fnGraph.createFromMesh(mesh);

		require(fnGraph.numVertices() == 4, "converted graph vertex count");
		require(fnGraph.numEdges() == 4, "converted graph edge count");
	}

	void testPointCloud()
	{
		zObjectPointCloud points;
		zFnPointCloud fnPoints(points);
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 2, 3)
		};
		fnPoints.create(positions);

		require(fnPoints.numVertices() == 2, "point-cloud vertex count");

		zItPointCloudVertex vertex(points, 1);
		zPoint updated(2, 3, 4);
		vertex.setPosition(updated);
		require(vertex.getPosition().z == 4, "point-cloud vertex edit");

		zObjectPointCloud copy = points;
		zFnPointCloud fnCopy(copy);
		require(fnCopy.numVertices() == 2, "point-cloud copy count");

		int activeCount = 0;
		for (zItPointCloudVertex it(copy); !it.end(); it++)
			if (it.isActive()) ++activeCount;
		require(activeCount == 2, "point-cloud iteration");
	}

	void testTransformationMatrixCopy()
	{
		zTransformationMatrix original;
		zFloat4 translation;
		translation[0] = 1.0f;
		translation[1] = 2.0f;
		translation[2] = 3.0f;
		translation[3] = 1.0f;
		original.setTranslation(translation);

		zTransformationMatrix copy = original;
		translation[0] = 4.0f;
		copy.setTranslation(translation);

		require(original.getTranslation().x == 1.0f, "transform copy preserves source");
		require(copy.getTranslation().x == 4.0f, "transform copy owns independent state");
	}

}

int main()
{
	try
	{
		zSpace::zObjectMesh mesh;
		zSpace::zObjectGraph graph;

		testMesh(mesh);
		testGraph(graph);
		testMeshToGraph(mesh);
		testPointCloud();
		testTransformationMatrixCopy();

		std::cout << "zspace smoke tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "zspace smoke tests failed: " << error.what() << '\n';
		return 1;
	}
}
