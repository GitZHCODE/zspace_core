// Bindings for zFnMesh - the mesh function set, where almost all mesh
// functionality actually lives (zObjMesh itself is an opaque handle).
//
// Scope note: methods whose signature requires a half-edge-mesh iterator
// type (zItMeshVertex/zItMeshEdge/zItMeshHalfEdge/zItMeshFace) as an
// in/out parameter are intentionally NOT bound here, since those iterator
// classes are outside the requested binding scope (zObjMesh/zFnMesh/
// zObjGraph/zFnGraph only). This affects: addVertex, addEdges, addPolygon
// (all overloads), updatePolygon, vertexExists, the iterator overload of
// halfEdgeExists, getEdgeCotangentWeight, computeEdgeLoop*, deleteEdge,
// collapseEdge, splitEdge, splitHalfEdge, flipTriangleEdge, faceTriangulate.
// Binding zIt* would be a natural follow-up if needed.

#include "numpy_arrays.h"
#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <zspace/zInterface/functionsets/zFnMesh.h>
#include <zspace/zInterface/objects/zObjGraph.h>
#include <zspace/zInterface/objects/zObjMesh.h>

namespace py = pybind11;
using namespace zSpace;

namespace zspace_python {

void bind_fn_mesh(py::module_ &m) {
  py::class_<zFnMesh>(m, "zFnMesh")
      .def(py::init<>())
      .def(py::init<zObjMesh &>(), py::arg("mesh_obj"), py::keep_alive<1, 2>())

      .def("setObject", &zFnMesh::setObject, py::arg("mesh_obj"), py::keep_alive<1, 2>())
      .def("object", &zFnMesh::object, py::return_value_policy::reference)
      .def("hasObject", &zFnMesh::hasObject)

      .def("getType", &zFnMesh::getType)
      .def("getBounds",
           [](zFnMesh &f) {
             zPoint minBB, maxBB;
             f.getBounds(minBB, maxBB);
             return std::make_tuple(minBB, maxBB);
           })
      .def("create_numpy", [](zFnMesh &f, const arrays::Floats &positions, const py::array &counts, const py::array &connects) {
        auto p=arrays::points(positions); auto c=arrays::integers(counts); auto ids=arrays::integers(connects);
        size_t total=0; for(int n:c) { if(n<3) throw py::value_error("Faces require at least three vertices"); total+=n; }
        if(total!=ids.size()) throw py::value_error("Face counts do not match connectivity");
        arrays::indices(ids,p.size()); f.create(p,c,ids);
      }, py::arg("positions"),py::arg("face_counts"),py::arg("face_indices"))
      .def("get_mesh_data_numpy", [](zFnMesh &f) {
        zPointArray p; zIntArray ids,c; f.getVertexPositions(p); f.getPolygonData(ids,c);
        return py::make_tuple(arrays::xyz(p),arrays::scalar(c),arrays::scalar(ids));
      })
      .def("get_positions_numpy", [](zFnMesh &f) { zPointArray p; f.getVertexPositions(p); return arrays::xyz(p); })
      .def("set_positions_numpy", [](zFnMesh &f, const arrays::Floats &a) {
        auto p=arrays::points(a); if(p.size()!=static_cast<size_t>(f.numVertices())) throw py::value_error("Vertex count mismatch"); f.setVertexPositions(p);
      },py::arg("positions"))
      .def("clear", &zFnMesh::clear)

      // ---- create -------------------------------------------------
      .def("reserve", &zFnMesh::reserve, py::arg("num_vertices"),
           py::arg("num_edges"), py::arg("num_faces"))
      .def("create", &zFnMesh::create, py::arg("positions"),
           py::arg("poly_counts"), py::arg("poly_connects"))

      // ---- topology query ------------------------------------------
      .def("numVertices", &zFnMesh::numVertices)
      .def("numEdges", &zFnMesh::numEdges)

      .def("numPolygons", &zFnMesh::numPolygons)

      .def("getTopologicalLaplacian", &zFnMesh::getTopologicalLaplacian)
      .def("checkPointInConvexHull", &zFnMesh::checkPointInConvexHull)

      // ---- compute --------------------------------------------------
      .def("computeEdgeColorfromVertexColor",
           &zFnMesh::computeEdgeColorfromVertexColor)
      .def("computeVertexColorfromEdgeColor",
           &zFnMesh::computeVertexColorfromEdgeColor)
      .def("computeFaceColorfromVertexColor",
           &zFnMesh::computeFaceColorfromVertexColor)
      .def("computeVertexColorfromFaceColor",
           &zFnMesh::computeVertexColorfromFaceColor)
      .def("smoothColors", &zFnMesh::smoothColors, py::arg("smooth_val") = 1,
           py::arg("type") = zVertexData)
      .def("computeVertexNormalfromFaceNormal",
           &zFnMesh::computeVertexNormalfromFaceNormal)
      .def("computeMeshNormals", &zFnMesh::computeMeshNormals)
      .def("averageVertices", &zFnMesh::averageVertices,
           py::arg("num_steps") = 1)


      .def("makeConvexHull", &zFnMesh::makeConvexHull, py::arg("points"))
      .def("isTriMesh", &zFnMesh::isTriMesh)
      .def("isQuadMesh", &zFnMesh::isQuadMesh)

      // ---- set --------------------------------------------------------
      .def("setVertexPositions", &zFnMesh::setVertexPositions, py::arg("positions"))
      .def("setVertexColor", &zFnMesh::setVertexColor, py::arg("color"),
           py::arg("set_face_color") = false)
      .def("setVertexColorsfromScalars", &zFnMesh::setVertexColorsfromScalars,
           py::arg("vertex_scalars"), py::arg("set_face_color") = false)
      .def("setVertexColors", &zFnMesh::setVertexColors, py::arg("colors"),
           py::arg("set_face_color") = false)
      .def("setVertexWeight", &zFnMesh::setVertexWeight, py::arg("weight"))
      .def("setVertexWeights", &zFnMesh::setVertexWeights, py::arg("weights"))
      .def("setFaceColor", &zFnMesh::setFaceColor, py::arg("color"),
           py::arg("set_vertex_color") = false)
      .def("setFaceColors", &zFnMesh::setFaceColors, py::arg("colors"),
           py::arg("set_vertex_color") = false)
      .def("setFaceColorOcclusion", &zFnMesh::setFaceColorOcclusion,
           py::arg("light_vector"), py::arg("set_vertex_color") = false)
      .def("setFaceNormals",
           py::overload_cast<zVector &>(&zFnMesh::setFaceNormals),
           py::arg("normal"))
      .def("setFaceNormals",
           py::overload_cast<zVectorArray &>(&zFnMesh::setFaceNormals),
           py::arg("normals"))
      .def("setEdgeColor", &zFnMesh::setEdgeColor, py::arg("color"),
           py::arg("set_vertex_color") = false)
      .def("setEdgeColors", &zFnMesh::setEdgeColors, py::arg("colors"),
           py::arg("set_vertex_color"))
      .def("setEdgeWeight", py::overload_cast<double>(&zFnMesh::setEdgeWeight),
           py::arg("weight"))
      .def("setEdgeWeight",
           py::overload_cast<int, double>(&zFnMesh::setEdgeWeight),
           py::arg("index"), py::arg("weight"))
      .def("setEdgeWeights", &zFnMesh::setEdgeWeights, py::arg("weights"))

      // ---- get ----------------------------------------------------------
      .def("getVertexPositions",
           [](zFnMesh &f, bool exclude_corner_vertices) {
             zPointArray pos;
             f.getVertexPositions(pos, exclude_corner_vertices);
             return pos;
           },
           py::arg("exclude_corner_vertices") = false)
      .def("getVertexNormals",
           [](zFnMesh &f) {
             zVectorArray n;
             f.getVertexNormals(n);
             return n;
           })
      .def("getVertexColors",
           [](zFnMesh &f) {
             zColorArray c;
             f.getVertexColors(c);
             return c;
           })
      .def("getVertexWeights",
           [](zFnMesh &f) {
             zDoubleArray w;
             f.getVertexWeights(w);
             return w;
           })
      .def("getEdgeColors",
           [](zFnMesh &f) {
             zColorArray c;
             f.getEdgeColors(c);
             return c;
           })
      .def("getEdgeWeights",
           [](zFnMesh &f) {
             zDoubleArray w;
             f.getEdgeWeights(w);
             return w;
           })
      .def("getFaceNormals",
           [](zFnMesh &f) {
             zVectorArray n;
             f.getFaceNormals(n);
             return n;
           })
      .def("getFaceColors",
           [](zFnMesh &f) {
             zColorArray c;
             f.getFaceColors(c);
             return c;
           })
      .def("getCenter", &zFnMesh::getCenter)
      .def("getCenters",
           [](zFnMesh &f, zHEData type) {
             zPointArray centers;
             f.getCenters(type, centers);
             return centers;
           },
           py::arg("type"))
      .def("getDualMesh",
           [](zFnMesh &f, bool exclude_boundary, bool keep_existing_boundary,
              bool rotate90) {
             zObjMesh dualMeshObj;
             zIntArray inEdge_dualEdge, dualEdge_inEdge;
             f.getDualMesh(dualMeshObj, inEdge_dualEdge, dualEdge_inEdge,
                           exclude_boundary, keep_existing_boundary, rotate90);
             return std::make_tuple(dualMeshObj, inEdge_dualEdge,
                                     dualEdge_inEdge);
           },
           py::arg("exclude_boundary"), py::arg("keep_existing_boundary") = false,
           py::arg("rotate90") = false)
      .def("getDualGraph",
           [](zFnMesh &f, bool exclude_boundary, bool planar_mesh,
              bool rotate90) {
             zObjGraph dualGraphObj;
             zIntArray inEdge_dualEdge, dualEdge_inEdge;
             f.getDualGraph(dualGraphObj, inEdge_dualEdge, dualEdge_inEdge,
                            exclude_boundary, planar_mesh, rotate90);
             return std::make_tuple(dualGraphObj, inEdge_dualEdge,
                                     dualEdge_inEdge);
           },
           py::arg("exclude_boundary") = false, py::arg("planar_mesh") = false,
           py::arg("rotate90") = false)
      .def("getRainflowGraph",
           [](zFnMesh &f, bool exclude_boundary) {
             zObjGraph rainflowGraphObj;
             f.getRainflowGraph(rainflowGraphObj, exclude_boundary);
             return rainflowGraphObj;
           },
           py::arg("exclude_boundary") = false)
      .def("getMeshTriangles",
           [](zFnMesh &f) {
             std::vector<zIntArray> faceTris;
             f.getMeshTriangles(faceTris);
             return faceTris;
           })
      .def("getMeshVolume", &zFnMesh::getMeshVolume)
      .def("getMeshFaceVolumes",
           [](zFnMesh &f, std::vector<zIntArray> faceTris, zPointArray fCenters,
              bool absolute_volumes) {
             zDoubleArray faceVolumes;
             f.getMeshFaceVolumes(faceTris, fCenters, faceVolumes,
                                  absolute_volumes);
             return std::make_tuple(faceTris, fCenters, faceVolumes);
           },
           py::arg("face_tris") = std::vector<zIntArray>{},
           py::arg("face_centers") = zPointArray{},
           py::arg("absolute_volumes") = true)
      .def("getPrincipalCurvatures",
           [](zFnMesh &f) {
             zCurvatureArray curvatures;
             zVectorArray pVector1, pVector2;
             f.getPrincipalCurvatures(curvatures, pVector1, pVector2);
             return std::make_tuple(curvatures, pVector1, pVector2);
           })
      .def("getGaussianCurvature",
           [](zFnMesh &f) {
             zDoubleArray curvatures;
             f.getGaussianCurvature(curvatures);
             return curvatures;
           })
      .def("getPlanarityDeviationPerFace",
           [](zFnMesh &f, zPlanarSolverType type, bool color_faces,
              double tolerance) {
             zDoubleArray devs;
             f.getPlanarityDeviationPerFace(devs, type, color_faces, tolerance);
             return devs;
           },
           py::arg("type"), py::arg("color_faces") = false,
           py::arg("tolerance") = EPS)
      .def("getEdgeDihedralAngles",
           [](zFnMesh &f) {
             zDoubleArray angles;
             f.getEdgeDihedralAngles(angles);
             return angles;
           })
      .def("getHalfEdgeLengths",
           [](zFnMesh &f) {
             zDoubleArray lengths;
             double total = f.getHalfEdgeLengths(lengths);
             return std::make_tuple(lengths, total);
           })
      .def("getEdgeLengths",
           [](zFnMesh &f) {
             zDoubleArray lengths;
             double total = f.getEdgeLengths(lengths);
             return std::make_tuple(lengths, total);
           })
      .def("getVertexAreas",
           [](zFnMesh &f, zPointArray faceCenters, zPointArray edgeCenters) {
             zFloatArray vertexAreas;
             double total = f.getVertexAreas(faceCenters, edgeCenters, vertexAreas);
             return std::make_tuple(vertexAreas, total);
           },
           py::arg("face_centers"), py::arg("edge_centers"))
      .def("getPlanarFaceAreas",
           [](zFnMesh &f) {
             zDoubleArray areas;
             double total = f.getPlanarFaceAreas(areas);
             return std::make_tuple(areas, total);
           })
      .def("getPolygonData",
           [](zFnMesh &f) {
             zIntArray polyConnects, polyCounts;
             f.getPolygonData(polyConnects, polyCounts);
             return std::make_tuple(polyConnects, polyCounts);
           })
      .def("getMatrices_trimesh",
           [](zFnMesh &f) {
             Eigen::MatrixXd V;
             Eigen::MatrixXi F;
             f.getMatrices_trimesh(V, F);
             return std::make_tuple(V, F);
           })
      .def("getMatrices_quadmesh",
           [](zFnMesh &f) {
             Eigen::MatrixXd V;
             Eigen::MatrixXi F;
             f.getMatrices_quadmesh(V, F);
             return std::make_tuple(V, F);
           })
      .def("getEdgeData",
           [](zFnMesh &f, bool exclude_boundary) {
             zIntArray edgeConnects;
             f.getEdgeData(edgeConnects, exclude_boundary);
             return edgeConnects;
           },
           py::arg("exclude_boundary") = false)
      .def("getDuplicate",
           [](zFnMesh &f) {
             zObjMesh out;
             f.getDuplicate(out);
             return out;
           })

      // ---- contour --------------------------------------------------
      .def("splitMesh_Mixed",
           [](zFnMesh &f, zPointArray &splitPlaneOrigins,
              zVectorArray &splitPlaneNormals) {
             zObjMesh result;
             f.splitMesh_Mixed(splitPlaneOrigins, splitPlaneNormals, result);
             return result;
           },
           py::arg("split_plane_origins"), py::arg("split_plane_normals"))
      .def("splitMesh_Quad",
           [](zFnMesh &f, std::vector<zPlane> &splitPlanes, bool invertMesh) {
             zObjMesh result;
             f.splitMesh_Quad(splitPlanes, invertMesh, result);
             return result;
           },
           py::arg("split_planes"), py::arg("invert_mesh"))
      .def("getIsoContour",
           [](zFnMesh &f, zScalarArray &vertexScalars, float threshold,
              int precision, float distTolerance, bool selectedFaces,
              zColor selectedFaceColor) {
             zPointArray positions;
             zIntArray edgeConnects;
             zColorArray cVertexColor;
             f.getIsoContour(vertexScalars, threshold, positions, edgeConnects,
                             cVertexColor, precision, distTolerance,
                             selectedFaces, selectedFaceColor);
             return std::make_tuple(positions, edgeConnects, cVertexColor);
           },
           py::arg("vertex_scalars"), py::arg("threshold"),
           py::arg("precision") = PRECISION,
           py::arg("dist_tolerance") = distanceTolerance,
           py::arg("selected_faces") = false,
           py::arg("selected_face_color") = zColor())
      .def("getIsoMesh",
           [](zFnMesh &f, zScalarArray &vertexScalars, float threshold,
              bool invertMesh) {
             zObjMesh out;
             f.getIsoMesh(vertexScalars, threshold, invertMesh, out);
             return out;
           },
           py::arg("vertex_scalars"), py::arg("threshold"), py::arg("invert_mesh"))
      .def("getIsoMesh_mixed",
           [](zFnMesh &f, zScalarArray &vertexScalars, float threshold,
              bool invertMesh) {
             zObjMesh out;
             f.getIsoMesh_mixed(vertexScalars, threshold, invertMesh, out);
             return out;
           },
           py::arg("vertex_scalars"), py::arg("threshold"), py::arg("invert_mesh"))
      .def("getIsobandMesh",
           [](zFnMesh &f, zScalarArray &vertexScalars, float thresholdLow,
              float thresholdHigh) {
             zObjMesh out;
             f.getIsobandMesh(vertexScalars, thresholdLow, thresholdHigh, out);
             return out;
           },
           py::arg("vertex_scalars"), py::arg("threshold_low"),
           py::arg("threshold_high"))

      // ---- tri-mesh modifier ------------------------------------------
      .def("triangulate", &zFnMesh::triangulate)

      // ---- delete modifier ----------------------------------------------


      // ---- topology modifier ---------------------------------------------



      .def("subdivide", &zFnMesh::subdivide, py::arg("num_divisions"))
      .def("smoothMesh", &zFnMesh::smoothMesh, py::arg("num_divisions") = 1,
           py::arg("smooth_corner") = false)
      .def("extrudeMesh",
           [](zFnMesh &f, float thickness, bool thicknessTris) {
             zObjMesh out;
             f.extrudeMesh(thickness, out, thicknessTris);
             return out;
           },
           py::arg("extrude_thickness"), py::arg("thickness_tris") = false)
      .def("extrudeVariableMesh",
           [](zFnMesh &f, zFloatArray extrudeThickness, bool bothSides,
              bool thicknessTris) {
             zObjMesh out;
             f.extrudeVariableMesh(extrudeThickness, out, bothSides,
                                   thicknessTris);
             return out;
           },
           py::arg("extrude_thickness"), py::arg("both_sides"),
           py::arg("thickness_tris") = false)
      .def("extrudeBoundaryEdge",
           [](zFnMesh &f, float thickness, bool thicknessTris) {
             zObjMesh out;
             f.extrudeBoundaryEdge(thickness, out, thicknessTris);
             return out;
           },
           py::arg("extrude_thickness"), py::arg("thickness_tris") = false)
      // NOTE: zFnMesh::extrude(float, bool) is declared in the SDK header
      // but not exported by this build of zSpace_Core.lib (link error) -
      // use extrudeMesh() above instead, which is exported and equivalent.

      // ---- transform overrides -------------------------------------------
      .def("setTransform", &zFnMesh::setTransform, py::arg("transform"),
           py::arg("decompose") = true, py::arg("update_positions") = true)
      .def("setScale",
           [](zFnMesh &f, std::array<float, 4> scale) {
             zFloat4 s{scale[0], scale[1], scale[2], scale[3]};
             f.setScale(s);
           },
           py::arg("scale"))
      .def("setRotation",
           [](zFnMesh &f, std::array<float, 4> rotation, bool appendRotations) {
             zFloat4 r{rotation[0], rotation[1], rotation[2], rotation[3]};
             f.setRotation(r, appendRotations);
           },
           py::arg("rotation"), py::arg("append_rotations") = false)
      .def("setTranslation", &zFnMesh::setTranslation, py::arg("translation"),
           py::arg("append_translations") = false)
      .def("setPivot", &zFnMesh::setPivot, py::arg("pivot"))
      .def("getTransform", [](zFnMesh &f) {
        zTransform t;
        f.getTransform(t);
        return t;
      });
}

}  // namespace zspace_python
