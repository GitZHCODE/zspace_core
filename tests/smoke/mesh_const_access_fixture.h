#pragma once

void testMeshConstAccess()
{
	using namespace zSpace;
	zObjectMesh mesh;
	struct QueryMesh : zFnMesh {
		using zFnMesh::zFnMesh;
		using zFnMesh::halfEdgeExists;
	};
	QueryMesh fn(mesh);
	zPointArray points = {zPoint(0,0,0), zPoint(1,0,0), zPoint(1,1,0), zPoint(0,1,0)};
	zIntArray counts = {3,3}, connects = {0,1,2,0,2,3};
	fn.create(points, counts, connects);
	const zObjectMesh& input = mesh;
	require(!zMeshObjectStorage::hasTopology(input), "fixture starts with cold topology");
	require(zItMeshHalfEdge(mesh).size() == 10, "two triangles have ten directed edges");
	const auto* topology = zMeshObjectStorage::readTopologyIfPresent(input);
	auto clean = [&]() {
		require(!zMeshObjectStorage::needsFaceListSync(input), "query must not mark face-list dirty");
		require(zMeshObjectStorage::readTopologyIfPresent(input) == topology, "query reuses topology");
	};
	clean();
	for (int pass = 0; pass < 3; ++pass) {
		int id = -1;
		require(fn.halfEdgeExists(0, 2, id), "shared diagonal lookup"); clean();
		require(!fn.halfEdgeExists(1, 3, id) && id == -1, "missing edge lookup"); clean();
		for (zItMeshHalfEdge he(mesh); !he.end(); he++) {
			clean();
			require(he.isActive(), "active half-edge"); clean();
			require(he.getSym().getSym().getId() == he.getId(), "opposite involution"); clean();
			require(he.getNext().getPrev().getId() == he.getId(), "next/previous cycle"); clean();
			he.size(); he.getVertex(); he.getStartVertex(); he.getEdge(); if (!he.onBoundary()) he.getFace(); clean();
			zPointArray p; he.getVertexPositions(p);
			he.getCenter(); he.getVector(); he.getLength(); he.getColor(); he.onBoundary(); clean();
		}
		for (zItMeshVertex v(mesh); !v.end(); v++) {
			zIntArray ids; zItMeshHalfEdgeArray edges;
			v.getConnectedHalfEdges(ids); v.getConnectedHalfEdges(edges); v.getHalfEdge(); clean();
		}
		for (zItMeshEdge e(mesh); !e.end(); e++) { e.getHalfEdge(0); e.getDihedralAngle(); clean(); }
		for (zItMeshFace f(mesh); !f.end(); f++) {
			zItMeshHalfEdgeArray edges; f.getHalfEdges(edges); f.getHalfEdge(); clean();
		}
	}

	zPointArray faceCenters, edgeCenters;
	fn.getCenters(zFaceData, faceCenters);
	for (zItMeshHalfEdge he(mesh); !he.end(); he++) edgeCenters.push_back(he.getCenter());
	zFloatArray areas;
	require(std::abs(fn.getVertexAreas(faceCenters, edgeCenters, areas) - 1.0) < 1e-6,
		"vertex areas sum to unit square area"); clean();

	// Closed topology exercises vertex area and mixed input-read/output-write code.
	zObjectMesh tetra;
	zFnMesh tetraFn(tetra);
	zPointArray tp = {zPoint(0,0,0), zPoint(1,0,0), zPoint(0,1,0), zPoint(0,0,1)};
	zIntArray tc = {3,3,3,3}, ti = {0,2,1,0,1,3,1,2,3,2,0,3};
	tetraFn.create(tp, tc, ti);
	for (zItMeshVertex v(tetra); !v.end(); v++) {
		require(v.getArea() > 0, "closed vertex area is positive");
		require(!zMeshObjectStorage::needsFaceListSync(tetra), "area query keeps source clean");
	}
	for (bool rotate : {false, true}) {
		zObjectMesh dual;
		zIntArray forward, reverse;
		tetraFn.getDualMesh(dual, forward, reverse, true, false, rotate);
		require(!zMeshObjectStorage::needsFaceListSync(tetra), "dual mesh keeps source clean");
		require(zFnMesh(dual).numVertices() == 4 && zFnMesh(dual).numPolygons() == 4,
			"tetrahedral dual has four vertices and faces");
		require(forward.size() == 12 && reverse.size() == 12, "dual maps cover directed edges");
		zObjectGraph graph;
		tetraFn.getDualGraph(graph, forward, reverse, true, false, rotate);
		require(!zMeshObjectStorage::needsFaceListSync(tetra), "dual graph keeps source clean");
		require(zFnGraph(graph).numVertices() == 4, "dual graph has one vertex per source face");
	}

	// A writable topology view must still invalidate and synchronize attributes.
	zMeshObjectStorage::get(mesh).vertexPositions[0].z = 2;
	require(zMeshObjectStorage::needsFaceListSync(input), "real topology edit marks dirty");
	require(zItMeshVertex(mesh, 0).getPosition().z == 2, "face-list receives topology edit");
	clean();
	// Face-list setters must still discard stale topology and rebuild on demand.
	zPoint moved(0,0,3);
	zItMeshVertex(mesh, 0).setPosition(moved);
	require(!zMeshObjectStorage::hasTopology(input), "position edit invalidates topology");
	require(zItMeshHalfEdge(mesh).size() == 10, "topology reconstructs after edit");
	topology = zMeshObjectStorage::readTopologyIfPresent(input);
	require(topology->vertexPositions[0].z == 3, "rebuilt topology receives position edit"); clean();
}
