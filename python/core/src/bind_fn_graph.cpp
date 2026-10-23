// Bindings for zFnGraph - the graph function set (zObjGraph itself is an
// opaque handle, see bind_obj_graph.cpp).
//
// Scope note: as in bind_fn_mesh.cpp, methods needing a half-edge-graph
// iterator type (zItGraphVertex/zItGraphEdge/zItGraphHalfEdge) as an in/out
// parameter are not bound: addVertex, addEdges, vertexExists, the iterator
// overload of halfEdgeExists, splitEdge, getGraphEccentricityCenter.

#include "numpy_arrays.h"
#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <zspace/zInterface/functionsets/zFnGraph.h>
#include <zspace/zInterface/objects/zObjGraph.h>
#include <zspace/zInterface/objects/zObjMesh.h>

namespace py = pybind11;
using namespace zSpace;

namespace zspace_python {

void bind_fn_graph(py::module_ &m) {
  py::class_<zFnGraph>(m, "zFnGraph")
      .def(py::init<>())
      .def(py::init<zObjGraph &, bool, zVector>(), py::arg("graph_obj"),
           py::arg("planar_graph") = false,
           py::arg("graph_normal") = zVector(0, 0, 1), py::keep_alive<1, 2>())

      .def_readwrite("planarGraph", &zFnGraph::planarGraph)
      .def_readwrite("graphNormal", &zFnGraph::graphNormal)

      .def("setObject", &zFnGraph::setObject, py::arg("graph_obj"), py::keep_alive<1, 2>())
      .def("object", &zFnGraph::object, py::return_value_policy::reference)
      .def("hasObject", &zFnGraph::hasObject)

      .def("getType", &zFnGraph::getType)
      .def("getBounds",
           [](zFnGraph &f) {
             zPoint minBB, maxBB;
             f.getBounds(minBB, maxBB);
             return std::make_tuple(minBB, maxBB);
           })
      .def("create_numpy", [](zFnGraph &f, const arrays::Floats &positions, const py::array &edges, int precision) {
        auto p=arrays::points(positions); auto ids=arrays::integers(edges);
        if(ids.size()%2) throw py::value_error("Edges require pairs of indices");
        arrays::indices(ids,p.size()); f.create(p,ids,precision);
      },py::arg("positions"),py::arg("edge_indices"),py::arg("precision")=PRECISION)
      .def("get_graph_data_numpy", [](zFnGraph &f) {
        zPointArray p; zIntArray ids; f.getVertexPositions(p); f.getEdgeData(ids);
        return py::make_tuple(arrays::xyz(p),arrays::scalar(ids));
      })
      .def("get_positions_numpy", [](zFnGraph &f) { zPointArray p; f.getVertexPositions(p); return arrays::xyz(p); })
      .def("set_positions_numpy", [](zFnGraph &f, const arrays::Floats &a) {
        auto p=arrays::points(a); if(p.size()!=static_cast<size_t>(f.numVertices())) throw py::value_error("Vertex count mismatch"); f.setVertexPositions(p);
      },py::arg("positions"))
      .def("clear", &zFnGraph::clear)

      // ---- create -----------------------------------------------------
      .def("create",
           py::overload_cast<zPointArray &, zIntArray &, int>(
               &zFnGraph::create),
           py::arg("positions"), py::arg("edge_connects"),
           py::arg("precision") = PRECISION)
      .def("create",
           py::overload_cast<zPointArray &, zIntArray &, zVector &>(
               &zFnGraph::create),
           py::arg("positions"), py::arg("edge_connects"),
           py::arg("graph_normal"))
      .def("createFromMesh", &zFnGraph::createFromMesh, py::arg("mesh_obj"),
           py::arg("exclude_boundary") = false)

      // ---- topology query -----------------------------------------------
      .def("numVertices", &zFnGraph::numVertices)
      .def("numEdges", &zFnGraph::numEdges)


      // ---- compute --------------------------------------------------------
      .def("computeEdgeColorfromVertexColor",
           &zFnGraph::computeEdgeColorfromVertexColor)
      .def("computeVertexColorfromEdgeColor",
           &zFnGraph::computeVertexColorfromEdgeColor)
      .def("averageVertices", &zFnGraph::averageVertices,
           py::arg("num_steps") = 1)

      // ---- set ------------------------------------------------------------
      .def("setVertexPositions", &zFnGraph::setVertexPositions,
           py::arg("positions"))
      .def("setVertexColor", &zFnGraph::setVertexColor, py::arg("color"),
           py::arg("set_edge_color") = false)
      .def("setVertexColors", &zFnGraph::setVertexColors, py::arg("colors"),
           py::arg("set_edge_color") = false)
      .def("setEdgeColor", &zFnGraph::setEdgeColor, py::arg("color"),
           py::arg("set_vertex_color") = false)
      .def("setEdgeColors", &zFnGraph::setEdgeColors, py::arg("colors"),
           py::arg("set_vertex_color"))
      .def("setEdgeWeight", &zFnGraph::setEdgeWeight, py::arg("weight"))
      .def("setEdgeWeights", &zFnGraph::setEdgeWeights, py::arg("weights"))

      // ---- get --------------------------------------------------------------
      .def("getVertexPositions",
           [](zFnGraph &f) {
             zPointArray pos;
             f.getVertexPositions(pos);
             return pos;
           })
      .def("getVertexColors",
           [](zFnGraph &f) {
             zColorArray c;
             f.getVertexColors(c);
             return c;
           })
      .def("getVertexWeights",
           [](zFnGraph &f) {
             zDoubleArray w;
             f.getVertexWeights(w);
             return w;
           })
      .def("getEdgeColors",
           [](zFnGraph &f) {
             zColorArray c;
             f.getEdgeColors(c);
             return c;
           })
      .def("getEdgeWeights",
           [](zFnGraph &f) {
             zDoubleArray w;
             f.getEdgeWeights(w);
             return w;
           })
      .def("getCenter", &zFnGraph::getCenter)
      .def("getCenters",
           [](zFnGraph &f, zHEData type) {
             zPointArray centers;
             f.getCenters(type, centers);
             return centers;
           },
           py::arg("type"))
      .def("getHalfEdgeLengths",
           [](zFnGraph &f) {
             zDoubleArray lengths;
             double total = f.getHalfEdgeLengths(lengths);
             return std::make_tuple(lengths, total);
           })
      .def("getEdgeLengths",
           [](zFnGraph &f) {
             zDoubleArray lengths;
             double total = f.getEdgeLengths(lengths);
             return std::make_tuple(lengths, total);
           })
      .def("getEdgeData",
           [](zFnGraph &f) {
             zIntArray edgeConnects;
             f.getEdgeData(edgeConnects);
             return edgeConnects;
           })
      .def("getDuplicate", &zFnGraph::getDuplicate,
           py::arg("planar_graph") = false,
           py::arg("graph_normal") = zVector(0, 0, 1))
      .def("getGraphMesh",
           [](zFnGraph &f, double width, zVector graphNormal) {
             zObjMesh out;
             f.getGraphMesh(out, width, graphNormal);
             return out;
           },
           py::arg("width") = 0.5, py::arg("graph_normal") = zVector(0, 0, 1), py::keep_alive<1, 2>())

      // ---- transform overrides ---------------------------------------------
      .def("setTransform", &zFnGraph::setTransform, py::arg("transform"),
           py::arg("decompose") = true, py::arg("update_positions") = true)
      .def("setScale",
           [](zFnGraph &f, std::array<float, 4> scale) {
             zFloat4 s{scale[0], scale[1], scale[2], scale[3]};
             f.setScale(s);
           },
           py::arg("scale"))
      .def("setRotation",
           [](zFnGraph &f, std::array<float, 4> rotation,
              bool appendRotations) {
             zFloat4 r{rotation[0], rotation[1], rotation[2], rotation[3]};
             f.setRotation(r, appendRotations);
           },
           py::arg("rotation"), py::arg("append_rotations") = false)
      .def("setTranslation", &zFnGraph::setTranslation, py::arg("translation"),
           py::arg("append_translations") = false)
      .def("setPivot", &zFnGraph::setPivot, py::arg("pivot"))
      .def("getTransform", [](zFnGraph &f) {
        zTransform t;
        f.getTransform(t);
        return t;
      });
}

}  // namespace zspace_python
