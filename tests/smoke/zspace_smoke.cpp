#include <zspace/interface.h>
#include <zspace/zInterface/functionsets/zFnMeshDynamics.h>
#include <zspace/zInterface/functionsets/zFnGraphDynamics.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include "origami_curved_fixture.h"
#include <src/zInterface/objects/zMeshObjectStorage.h>

namespace
{
	using namespace zSpace;

	void require(bool condition, const char* message)
	{
		if (!condition) throw std::runtime_error(message);
	}

#include "mesh_iterator_geometry_fixture.h"
#include "mesh_const_access_fixture.h"

	void testDynamicsVertexLoads()
	{
		zPointArray points={zPoint(0,0,0),zPoint(1,0,0),zPoint(0,1,0)};
		zIntArray edges={0,1,1,2},counts={3},connects={0,1,2};
		zObjectGraph graph;zFnGraph(graph).create(points,edges);zFnGraphDynamics graphDynamics(graph);
		graphDynamics.makeDynamic(false);graphDynamics.makeDynamic(false);
		zVector load(0,0,1);graphDynamics.addLoadForce(2,1,load);graphDynamics.update(1,zEuler,true,true,true);
		auto gp=graphDynamics.getRawVertexPositions();
		require(gp[0].z==0&&gp[1].z==2&&gp[2].z==0,"graph load targets one persistent vertex");
		zIntArray fixed={1};graphDynamics.setFixed(fixed);graphDynamics.addLoadForce(2,1,load);graphDynamics.update(1,zEuler,true,true,true);
		require(gp[1].z==2,"graph dynamics preserves fixed vertices");
		zObjectMesh mesh;zFnMesh(mesh).create(points,counts,connects);zFnMeshDynamics meshDynamics(mesh);meshDynamics.makeDynamic(false);
		meshDynamics.addLoadForce(2,1,load);meshDynamics.update(1,zEuler,true,true,true);
		auto mp=meshDynamics.getRawVertexPositions();
		require(mp[0].z==0&&mp[1].z==2&&mp[2].z==0,"mesh load targets only the requested vertex");
		bool rejected=false;try{meshDynamics.addLoadForce(1,3,load);}catch(const std::out_of_range&){rejected=true;}
		require(rejected,"dynamics rejects an invalid vertex index");
	}

	void testOrigami()
	{
		zObjectMesh mesh;
		zFnMesh fn(mesh);
		zPointArray points = {zPoint(0,0,0), zPoint(1,0,0), zPoint(0,1,0), zPoint(1,-1,0)};
		zIntArray counts = {3,3}, connects = {0,1,2,1,0,3};
		fn.create(points, counts, connects);
		zFnMeshDynamics dynamics;
		dynamics.create(mesh, false);
		dynamics.prepareOrigami();
		zIntArray fixed = {0,1}; dynamics.setFixed(fixed);
		zIntArray edges; fn.getEdgeData(edges);
		for (int i=0; i<static_cast<int>(edges.size()); i+=2)
			if (edges[i]+edges[i+1]==1) dynamics.setOrigamiCrease(i/2,1,1.5707963267948966);
		zOrigamiSettings settings; settings.axial=100; settings.foldAmount=1;
		zOrigamiDiagnostics diagnostics;
		zVectorArray forces; zOrigamiForceComponents components;
		dynamics.getOrigamiForces(settings, forces, diagnostics, &components);
		for (std::size_t i=0;i<forces.size();++i) {
			zVector sum=components.axial[i]+components.crease[i]+components.facet[i]+components.face[i]+components.damping[i];
			sum-=forces[i];
			require(sum.length()<1e-5, "origami display components sum to resultant");
		}
		for (int i=0;i<12000;++i) dynamics.stepOrigami(settings,0.01,diagnostics);
		require(diagnostics.maxAngleError<0.02, "native origami reaches crease angle");
		require(diagnostics.maxStrain<0.01, "native origami preserves edge lengths");
		const auto p=fn.getRawVertexPositions();
		require(p[2].z>0.5 && p[3].z>0.5, "native valley direction");
		require(p[0].z==0 && p[1].z==0, "native origami fixed vertices");

		// With no forces, a rigid translation must consume the entire requested interval.
		zObjectMesh movingMesh; zFnMesh movingFn(movingMesh);
		movingFn.create(points, counts, connects);
		struct MovingFixture : zFnMeshDynamics { using zFnMeshDynamics::particlesObj; };
		MovingFixture moving; moving.create(movingMesh, false); moving.prepareOrigami();
		zOrigamiSettings freeMotion;
		freeMotion.axial=freeMotion.fold=freeMotion.facet=freeMotion.face=freeMotion.damping=0;
		for (auto& particle : moving.particlesObj) {
			zFnParticle particleFn(particle); zVector velocity(0,0,1);
			particleFn.setVelocity(velocity);
		}
		moving.stepOrigami(freeMotion,0.1,diagnostics);
		for (int i=0;i<4;++i)
			require(std::abs(movingFn.getRawVertexPositions()[i].z-0.1)<1e-6, "origami consumes full timestep beyond stability cap");
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
		int meshEdgeId = -1;
		require(fnMesh.edgeExists(0, 1, meshEdgeId) && meshEdgeId >= 0, "mesh edge exists");
		zDoubleArray weights = {2,3,4,5};
		fnMesh.setEdgeWeights(weights);
		zIntArray sourceEdges; fnMesh.getEdgeData(sourceEdges);
		auto& topology = zMeshObjectStorage::get(mesh);
		for (int i=0;i<topology.n_e;++i) {
			const int a=topology.halfEdges[topology.edges[i].getHalfEdge(0)].getVertex();
			const int b=topology.halfEdges[topology.edges[i].getHalfEdge(1)].getVertex();
			bool matched=false;
			for (std::size_t j=0;j<sourceEdges.size();j+=2) {
				if ((sourceEdges[j]==a&&sourceEdges[j+1]==b)||(sourceEdges[j]==b&&sourceEdges[j+1]==a)) {
					require(topology.edgeWeights[i]==weights[j/2], "topology preserves endpoint-mapped weights");
					matched=true;
				}
			}
			require(matched, "topology edge has a source edge");
		}

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
		int graphEdgeId = -1;
		require(fnGraph.edgeExists(1, 2, graphEdgeId) && graphEdgeId == 1, "graph edge exists");

		int edgeCount = 0;
		for (zItGraphEdge edge(graph); !edge.end(); edge++)
		{
			zIntArray edgeVertices;
			edge.getVertices(edgeVertices);
			require(edgeVertices.size() == 2, "graph edge-list iterator endpoints");
			edgeCount++;
		}
		require(edgeCount == 2, "graph edge-list iterator count");

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
			// Vertex 4 was moved above; obtain the current positions for this check.
			zPointArray current; fnMesh.getVertexPositions(current);
			auto expectedCenter = (current[vertices[0]] + current[vertices[1]]) * 0.5;
			require((edge.getCenter()-expectedCenter).length()<1e-6,
				"non-manifold edge center without topology");
			require(std::abs(edge.getLength()-current[vertices[0]].distanceTo(current[vertices[1]]))<1e-6,
				"non-manifold edge length without topology");
			edgeCount++;
		}
		require(edgeCount == 7, "non-manifold edge iteration");

		int faceCount = 0;
		for (zItMeshFace face(mesh); !face.end(); face++)
		{
			zIntArray vertices;
			face.getVertices(vertices);
			require(vertices.size() == 3, "non-manifold face vertices");
			require(face.getNumVertices()==3, "non-manifold face corner count without topology");
			zPointArray current; fnMesh.getVertexPositions(current);
			zVector expected;
			for (int id : vertices) expected += current[id];
			expected /= 3;
			require((face.getCenter()-expected).length()<1e-6, "non-manifold face center without topology");
			int numTris=0; zIntArray tris;
			face.getTriangles(numTris,tris);
			require(numTris==1 && tris==vertices, "non-manifold triangle iterator without topology");
			zVector center;
			require(face.getVolume(tris,center,false)==0,"non-manifold triangle volume without topology");
			zPointArray offsets; face.getOffsetFacePositions(.1,offsets);
			require(offsets.size()==3, "non-manifold face offset without topology");
			face.updateNormal();
			require(std::isfinite(face.getNormal().length()), "non-manifold face normal update without topology");
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

	void testMeshMigrationCoverage()
	{
		zObjectMesh mesh;
		zFnMesh fnMesh(mesh);
		zPointArray positions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(1, 1, 0),
			zPoint(0, 1, 0),
			zPoint(2, 0, 0),
			zPoint(2, 1, 0)
		};
		zIntArray polygonCounts = { 4, 4 };
		zIntArray polygonConnects = { 0, 1, 2, 3, 1, 4, 5, 2 };
		fnMesh.create(positions, polygonCounts, polygonConnects);

		require(fnMesh.isQuadMesh(), "face-list quad classification");
		require(!fnMesh.isTriMesh(), "face-list tri classification");

		zDoubleArray edgeLengths;
		require(std::abs(fnMesh.getEdgeLengths(edgeLengths) - 7.0) < 1.0e-6,
			"face-list edge lengths");
		require(edgeLengths.size() == 7, "face-list edge length count");

		zIntArray interiorEdges;
		fnMesh.getEdgeData(interiorEdges, true);
		require(interiorEdges.size() == 2, "face-list boundary-excluded edge data");

		zDoubleArray dihedralAngles;
		fnMesh.getEdgeDihedralAngles(dihedralAngles);
		require(dihedralAngles.size() == 7, "face-list dihedral angle count");

		zDoubleArray planarity;
		fnMesh.getPlanarityDeviationPerFace(planarity, zQuadPlanar);
		require(planarity.size() == 2 && planarity[0] < 1.0e-6 && planarity[1] < 1.0e-6,
			"face-list planarity deviation");

		zObjectMesh extruded;
		fnMesh.extrudeMesh(0.1f, extruded);
		zFnMesh fnExtruded(extruded);
		require(fnExtruded.numVertices() == 12, "face-list extrusion vertices");
		require(fnExtruded.numPolygons() == 10, "face-list extrusion faces");

		zObjectMesh tetra;
		zFnMesh fnTetra(tetra);
		zPointArray tetraPositions = {
			zPoint(0, 0, 0),
			zPoint(1, 0, 0),
			zPoint(0, 1, 0),
			zPoint(0, 0, 1)
		};
		zIntArray tetraCounts = { 3, 3, 3, 3 };
		zIntArray tetraConnects = {
			0, 2, 1,
			0, 1, 3,
			1, 2, 3,
			2, 0, 3
		};
		fnTetra.create(tetraPositions, tetraCounts, tetraConnects);

		zObjectGraph dualGraph;
		zIntArray inEdgeDualEdge;
		zIntArray dualEdgeInEdge;
		fnTetra.getDualGraph(dualGraph, inEdgeDualEdge, dualEdgeInEdge, true);
		zFnGraph fnDualGraph(dualGraph);
		require(fnDualGraph.numVertices() == 4, "topology dual graph vertices");
		require(fnDualGraph.numEdges() == 6, "topology dual graph edges");
	}

	void testFields()
	{
		zObjectMeshScalarField meshScalarField;
		zFnMeshScalarField meshScalarFn(meshScalarField);
		meshScalarFn.create(zPoint(0, 0, 0), zPoint(2, 2, 0), 3, 3);
		require(meshScalarFn.numFieldValues() == 9, "mesh scalar field value count");

		zScalarArray meshScalars(meshScalarFn.numFieldValues(), 1.0f);
		meshScalarFn.setFieldValues(meshScalars);
		zScalarArray readMeshScalars;
		meshScalarFn.getFieldValues(readMeshScalars);
		require(readMeshScalars.size() == meshScalars.size(), "mesh scalar field readback");

		zObjectMeshVectorField meshVectorField;
		zFnMeshVectorField meshVectorFn(meshVectorField);
		meshVectorFn.createVectorFromScalarField(meshScalarField);
		zVectorArray meshVectors;
		meshVectorFn.getFieldValues(meshVectors);
		require(meshVectors.size() == meshScalars.size(), "mesh vector field from scalar");

		zObjectPointScalarField pointScalarField;
		zFnPointScalarField pointScalarFn(pointScalarField);
		pointScalarFn.create(zPoint(0, 0, 0), zPoint(1, 1, 1), 2, 2, 2);
		require(pointScalarFn.numFieldValues() == 8, "point scalar field value count");

		zScalarArray pointScalars(pointScalarFn.numFieldValues(), 2.0f);
		pointScalarFn.setFieldValues(pointScalars);
		zScalarArray readPointScalars;
		pointScalarFn.getFieldValues(readPointScalars);
		require(readPointScalars.size() == pointScalars.size(), "point scalar field readback");

		zObjectPointVectorField pointVectorField;
		zFnPointVectorField pointVectorFn(pointVectorField);
		(void)pointVectorFn;
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

int main(int argc, char** argv)
{
	try
	{
		if (argc>=2) {testCurvedOrigamiInput(argv[1],argc>2?std::stod(argv[2]):.7,argc>3?std::stod(argv[3]):20);return 0;}
		zSpace::zObjectMesh mesh;
		zSpace::zObjectGraph graph;

		testMesh(mesh);
		testDynamicsVertexLoads();
		testOrigami();
		testGraph(graph);
		testMeshToGraph(mesh);
		testNonManifoldMesh();
		testFaceListMeshAlgorithms();
		testMeshMigrationCoverage();
		testMeshIteratorGeometry();
		testMeshConstAccess();
		testSmoothBatchAndBounds();
		testFlatGeometryTopologyMigration();
		testFields();
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
