// Bindings for zObjGraph - see bind_obj_mesh.cpp for the rationale
// (opaque pimpl object, functionality lives on zFnGraph).

#include <pybind11/pybind11.h>

#include <zspace/zInterface/objects/zObjGraph.h>

namespace py = pybind11;
using namespace zSpace;

namespace zspace_python {

void bind_obj_graph(py::module_ &m) {
  py::class_<zObjGraph>(m, "zObjGraph")
      .def(py::init<>())
      .def(py::init<const zObjGraph &>())
      .def("__repr__", [](const zObjGraph &) { return "<zspace.zObjGraph>"; });
}

}  // namespace zspace_python
