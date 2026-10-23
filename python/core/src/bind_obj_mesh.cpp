// Bindings for zObjMesh. The object classes in zSpace Core are opaque
// pimpl handles - almost all functionality lives on the paired function set
// (zFnMesh), so this binding is intentionally small: construct one, copy it,
// hand it to zFnMesh(...) or to zTs3DP/zSlicer.

#include <pybind11/pybind11.h>

#include <zspace/zInterface/objects/zObjMesh.h>

namespace py = pybind11;
using namespace zSpace;

namespace zspace_python {

void bind_obj_mesh(py::module_ &m) {
  py::class_<zObjMesh>(m, "zObjMesh")
      .def(py::init<>())
      .def(py::init<const zObjMesh &>())
      .def("__repr__", [](const zObjMesh &) { return "<zspace.zObjMesh>"; });
}

}  // namespace zspace_python
