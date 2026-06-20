#include <zspace/interface.h>

#include <cmath>
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

	void testNonManifoldMesh()
	{
		zObjectMesh mesh;
		zFnMesh fnMesh(mesh);
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(0.5, 1, 0),
			zPoint(0.5, -1, 0),
			zPoint(0.5, 0, 1)
		};
		zIntArray polygonCounts = { 3, 3, 3 };
		zIntArray polygonConnects = {
			0, 1, 2,
			1, 0, 3,
			0, 1, 4
		};
		fnMesh.create(positions, polygonCounts, polygonConnects);

		require(fnMesh.numVertices() == 5, "non-manifold vertex count");
		require(fnMesh.numPolygons() == 3, "non-manifold polygon count");
		require(fnMesh.numEdges() == 7, "non-manifold unique edge count");

		zIntArray exportedCounts;
		zIntArray exportedConnects;
		fnMesh.getPolygonData(exportedConnects, exportedCounts);
		require(exportedCounts == polygonCounts, "non-manifold polygon export");
		require(exportedConnects == polygonConnects, "non-manifold connectivity export");

		zVectorArray normals;
		fnMesh.getFaceNormals(normals);
		require(normals.size() == 3, "non-manifold face normals");

		int vertexCount = 0;
		for (zItMeshVertex vertex(mesh); !vertex.end(); vertex++) vertexCount++;
		require(vertexCount == 5, "non-manifold vertex iteration");
		zItMeshVertex vertex(mesh, 4);
		vertex.setPosition(zPoint(0.5, 0, 1.25));
		require(vertex.getPosition().z == 1.25, "non-manifold vertex iterator edit");

		int edgeCount = 0;
		for (zItMeshEdge edge(mesh); !edge.end(); edge++)
		{
			zIntArray vertices;
			edge.getVertices(vertices);
			require(vertices.size() == 2, "non-manifold edge endpoints");
			edgeCount++;
		}
		require(edgeCount == 7, "non-manifold edge iteration");

		int faceCount = 0;
		for (zItMeshFace face(mesh); !face.end(); face++)
		{
			zIntArray vertices;
			face.getVertices(vertices);
			require(vertices.size() == 3, "non-manifold face vertices");
			faceCount++;
		}
		require(faceCount == 3, "non-manifold face iteration");

		bool topologyRejected = false;
		try
		{
			zItMeshHalfEdge halfEdge(mesh, 0);
			(void)halfEdge;
		}
		catch (const std::runtime_error&)
		{
			topologyRejected = true;
		}
		require(topologyRejected, "non-manifold half-edge query is rejected");
	}

	void testFaceListMeshAlgorithms()
	{
		zObjectMesh mesh;
		zFnMesh fnMesh(mesh);
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(1, 1, 0),
			zPoint(0, 1, 0)
		};
		zIntArray polygonCounts = { 4 };
		zIntArray polygonConnects = { 0, 1, 2, 3 };
		fnMesh.create(positions, polygonCounts, polygonConnects);

		zPointArray centers;
		fnMesh.getCenters(zFaceData, centers);
		require(centers.size() == 1 && std::abs(centers[0].x - 0.5) < 1.0e-6,
			"face-list face center");

		zDoubleArray areas;
		require(std::abs(fnMesh.getPlanarFaceAreas(areas) - 1.0) < 1.0e-6,
			"face-list polygon area");

		std::vector<zIntArray> triangles;
		fnMesh.getMeshTriangles(triangles);
		require(triangles.size() == 1 && triangles[0].size() == 6,
			"face-list polygon triangulation data");

		zScalarArray scalars = { -1.0f, 1.0f, 1.0f, -1.0f };
		zPointArray contourPositions;
		zIntArray contourEdges;
		zColorArray contourColors;
		fnMesh.getIsoContour(scalars, 0.0f, contourPositions, contourEdges, contourColors);
		require(contourPositions.size() == 2 && contourEdges.size() == 2,
			"face-list isoline extraction");

		zObjectMesh isoMesh;
		fnMesh.getIsoMesh(scalars, 0.0f, false, isoMesh);
		zFnMesh fnIsoMesh(isoMesh);
		require(fnIsoMesh.numPolygons() == 1 && fnIsoMesh.numVertices() == 4,
			"face-list iso mesh clipping");

		zObjectMesh bandMesh;
		fnMesh.getIsobandMesh(scalars, -0.5f, 0.5f, bandMesh);
		zFnMesh fnBandMesh(bandMesh);
		require(fnBandMesh.numPolygons() == 1 && fnBandMesh.numVertices() == 4,
			"face-list isoband clipping");

		fnMesh.triangulate();
		fnMesh.getPolygonData(polygonConnects, polygonCounts);
		require(fnMesh.numPolygons() == 2 && polygonCounts == zIntArray({ 3, 3 }),
			"face-list mesh triangulation");
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
		testNonManifoldMesh();
		testFaceListMeshAlgorithms();
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
