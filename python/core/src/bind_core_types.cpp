// Bindings for zVector, zColor and the enums shared across zFnMesh/zFnGraph
// (zHEData, zFnType, zPlanarSolverType). These are value types used
// pervasively as parameters/return values by zObjMesh/zFnMesh/zObjGraph/
// zFnGraph, so they need to be registered before those classes are bound.

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// ZSPACE_STATIC_LIBRARY is defined for the whole target (see CMakeLists.txt)
// so these headers only declare classes; symbols resolve from zSpace_Core.lib.
#include <zspace/zCore/base/zColor.h>
#include <zspace/zCore/base/zDefinitions.h>
#include <zspace/zCore/base/zEnumerators.h>
#include <zspace/zCore/base/zMatrix.h>
#include <zspace/zCore/base/zVector.h>

namespace py = pybind11;
using namespace zSpace;

namespace zspace_python {

void bind_core_types(py::module_ &m) {
  // ---- zVector (zPoint is an alias for the same type) ------------------
  py::class_<zVector> vec(m, "zVector");
  vec.def(py::init<>())
      .def(py::init<float, float, float, float>(), py::arg("x"), py::arg("y"),
           py::arg("z"), py::arg("w") = 1.0f)
      .def_readwrite("x", &zVector::x)
      .def_readwrite("y", &zVector::y)
      .def_readwrite("z", &zVector::z)
      .def("__eq__", [](zVector &a, zVector &b) { return a == b; })
      .def("__ne__", [](zVector &a, zVector &b) { return a != b; })
      .def("__getitem__", [](zVector &v, int i) { return v[i]; })
      .def("__add__", [](zVector &a, const zVector &b) { return a + b; })
      .def("__sub__", [](zVector &a, const zVector &b) { return a - b; })
      .def("__mul__", [](zVector &a, const zVector &b) { return a * b; })  // dot product -> float
      .def("__xor__", [](zVector &a, const zVector &b) { return a ^ b; })  // cross product -> zVector
      .def("__add__", [](zVector &a, float v) { return a + v; })
      .def("__sub__", [](zVector &a, float v) { return a - v; })
      .def("__mul__", [](zVector &a, float v) { return a * v; })
      .def("__truediv__", [](zVector &a, float v) { return a / v; })
      .def("__mul__", [](zVector &a, const Eigen::Matrix3f &m) { return a * m; })
      .def("__mul__", [](zVector &a, const Eigen::Matrix4f &m) { return a * m; })
      .def("__iadd__", [](zVector &a, const zVector &b) { a += b; return a; })
      .def("__isub__", [](zVector &a, const zVector &b) { a -= b; return a; })
      .def("__iadd__", [](zVector &a, float v) { a += v; return a; })
      .def("__isub__", [](zVector &a, float v) { a -= v; return a; })
      .def("__imul__", [](zVector &a, float v) { a *= v; return a; })
      .def("__itruediv__", [](zVector &a, float v) { a /= v; return a; })
      .def("length2", &zVector::length2)
      .def("length", &zVector::length)
      .def("normalize", &zVector::normalize)
      .def("squareDistanceTo", &zVector::squareDistanceTo)
      .def("distanceTo", &zVector::distanceTo)
      .def("angle", &zVector::angle)
      .def("angle360", &zVector::angle360)
      .def("dihedralAngle", &zVector::dihedralAngle)
      .def("cotan", &zVector::cotan)
      .def("getComponents",
           [](zVector &v) {
             zFloat4 vals;
             v.getComponents(vals);
             return std::array<float, 4>{vals[0], vals[1], vals[2], vals[3]};
           })
      .def("rotateAboutAxis", &zVector::rotateAboutAxis, py::arg("axis"),
           py::arg("angle") = 0.0f)
      .def("__repr__", [](const zVector &v) {
        return "zVector(" + std::to_string(v.x) + ", " + std::to_string(v.y) +
               ", " + std::to_string(v.z) + ")";
      });

  // zPoint is a plain alias (typedef zVector zPoint;) - no separate binding.

  // ---- zColor ------------------------------------------------------------
  py::class_<zColor>(m, "zColor")
      .def(py::init<>())
      .def(py::init<float, float, float, float>(), py::arg("r"), py::arg("g"),
           py::arg("b"), py::arg("a"))
      .def(py::init<float, float, float>(), py::arg("h"), py::arg("s"),
           py::arg("v"))
      .def_readwrite("r", &zColor::r)
      .def_readwrite("g", &zColor::g)
      .def_readwrite("b", &zColor::b)
      .def_readwrite("a", &zColor::a)
      .def_readwrite("h", &zColor::h)
      .def_readwrite("s", &zColor::s)
      .def_readwrite("v", &zColor::v)
      .def("__eq__", [](zColor &a, zColor &b) { return a == b; })
      .def("toHSV", &zColor::toHSV)
      .def("toRGB", &zColor::toRGB)
      .def("__repr__", [](const zColor &c) {
        return "zColor(" + std::to_string(c.r) + ", " + std::to_string(c.g) +
               ", " + std::to_string(c.b) + ", " + std::to_string(c.a) + ")";
      });

  m.attr("zRED") = zRED;
  m.attr("zGREEN") = zGREEN;
  m.attr("zBLUE") = zBLUE;
  m.attr("zWHITE") = zWHITE;
  m.attr("zGREY") = zGREY;
  m.attr("zBLACK") = zBLACK;
  m.attr("zMAGENTA") = zMAGENTA;
  m.attr("zYELLOW") = zYELLOW;
  m.attr("zCYAN") = zCYAN;

  // ---- zCurvature (plain {k1, k2} pair, used by zFnMesh::getPrincipalCurvatures) ----
  py::class_<zCurvature>(m, "zCurvature")
      .def(py::init<>())
      .def_readwrite("k1", &zCurvature::k1)
      .def_readwrite("k2", &zCurvature::k2);

  // ---- Shared enums --------------------------------------------------
  py::enum_<zHEData>(m, "zHEData")
      .value("zVertexData", zVertexData)
      .value("zHalfEdgeData", zHalfEdgeData)
      .value("zEdgeData", zEdgeData)
      .value("zFaceData", zFaceData);

  py::enum_<zFnType>(m, "zFnType")
      .value("zInvalidFn", zInvalidFn)
      .value("zPointsFn", zPointsFn)
      .value("zMeshFn", zMeshFn)
      .value("zGraphFn", zGraphFn)
      .value("zMeshFieldFn", zMeshFieldFn)
      .value("zPointFieldFn", zPointFieldFn)
      .value("zParticleFn", zParticleFn)
      .value("zMeshDynamicsFn", zMeshDynamicsFn)
      .value("zGraphDynamicsFn", zGraphDynamicsFn)
      .value("zComputeMeshFn", zComputeMeshFn)
      .value("zNurbsCurveFn", zNurbsCurveFn)
      .value("zArcFn", zArcFn)
      .value("zPlaneFn", zPlaneFn);

  py::enum_<zPlanarSolverType>(m, "zPlanarSolverType")
      .value("zQuadPlanar", zQuadPlanar)
      .value("zVolumePlanar", zVolumePlanar);
}

}  // namespace zspace_python
