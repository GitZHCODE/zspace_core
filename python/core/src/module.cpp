#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#if defined(ZSPACE_PYTHON_WITH_USD)
#include <zspace/zIO/zUSDapi.h>
#endif

#include "bindings.h"
#include <zspace/zInterface/objects/zObjMeshField.h>

namespace py = pybind11;

#if defined(ZSPACE_PYTHON_WITH_USD)
namespace {
py::dict usdPrim(const zSpace::zUSDPrimInfo &p) {
  py::dict out;
  out["path"] = p.path;
  out["type"] = p.type;
  out["attributes"] = p.attributes;
  out["mirrored"] = p.mirrored;
  return out;
}
py::dict usdValue(const zSpace::zUSDValue &a) {
  py::dict out;
  out["type"] = a.type;
  out["value"] = std::visit(
      [](const auto &v) -> py::object { return py::cast(v); }, a.value);
  return out;
}
zSpace::zUSDValue usdValueFromPython(py::dict a) {
  zSpace::zUSDValue out;
  out.type = a["type"].cast<std::string>();
  const auto &t = out.type;
  py::handle v = a["value"];
  if (t == "bool") {
    if (!py::isinstance<py::bool_>(v))
      throw py::type_error("Expected bool.");
    out.value = v.cast<bool>();
  } else if (t == "int")
    out.value = v.cast<int>();
  else if (t == "uint")
    out.value = v.cast<unsigned int>();
  else if (t == "float")
    out.value = v.cast<float>();
  else if (t == "double")
    out.value = v.cast<double>();
  else if (t == "string" || t == "token")
    out.value = v.cast<std::string>();
  else if (t == "int[]")
    out.value = v.cast<std::vector<int>>();
  else if (t == "uint[]")
    out.value = v.cast<std::vector<unsigned int>>();
  else if (t == "float[]" || t == "point3f[]" || t == "normal3f[]" ||
           t == "vector3f[]" || t == "float3[]")
    out.value = v.cast<std::vector<float>>();
  else if (t == "double[]" || t == "point3d[]" || t == "normal3d[]" ||
           t == "vector3d[]" || t == "double3[]")
    out.value = v.cast<std::vector<double>>();
  else if (t == "string[]" || t == "token[]")
    out.value = v.cast<std::vector<std::string>>();
  else
    throw py::value_error("Unsupported USD value type: " + t);
  return out;
}
zSpace::zUSDEdits usdEdits(py::dict input) {
  zSpace::zUSDEdits edits;
  if (input.contains("remove"))
    edits.remove = input["remove"].cast<std::vector<std::string>>();
  if (input.contains("removeCustomLayerData"))
    edits.removeCustomLayerData =
        input["removeCustomLayerData"].cast<std::vector<std::string>>();
  if (input.contains("customLayerData"))
    for (auto entry : input["customLayerData"].cast<py::dict>())
      edits.customLayerData[py::cast<std::string>(entry.first)] =
          usdValueFromPython(py::cast<py::dict>(entry.second));
  if (input.contains("prims"))
    for (auto item : input["prims"].cast<py::list>()) {
      auto spec = py::cast<py::dict>(item);
      zSpace::zUSDPrimEdit prim;
      prim.path = spec["path"].cast<std::string>();
      prim.type = spec["type"].cast<std::string>();
      for (auto entry : spec["attributes"].cast<py::dict>()) {
        auto a = py::cast<py::dict>(entry.second);
        zSpace::zUSDAttribute attribute;
        static_cast<zSpace::zUSDValue &>(attribute) = usdValueFromPython(a);
        if (a.contains("custom"))
          attribute.custom = a["custom"].cast<bool>();
        if (a.contains("interpolation"))
          attribute.interpolation = a["interpolation"].cast<std::string>();
        prim.attributes[py::cast<std::string>(entry.first)] =
            std::move(attribute);
      }
      edits.prims.push_back(std::move(prim));
    }
  return edits;
}
} // namespace
#endif

PYBIND11_MODULE(_core, m) {
  m.doc() = "zSpace Core SDK bindings: zVector, zColor, zObjMesh/zFnMesh, "
            "zObjGraph/zFnGraph.";

  zspace_python::bind_core_types(m);
  zspace_python::bind_obj_mesh(m);
  zspace_python::bind_fn_mesh(m);
  zspace_python::bind_obj_graph(m);
  zspace_python::bind_fn_graph(m);
  py::class_<zSpace::zObjMeshScalarField>(m, "ObjMeshScalarField")
      .def(py::init<>());
#if defined(ZSPACE_PYTHON_WITH_USD)
  py::class_<zSpace::USDAttribute>(m, "USDAttribute")
      .def("isValid", &zSpace::USDAttribute::isValid)
      .def("getTypeName", &zSpace::USDAttribute::getTypeName)
      .def("get", [](const zSpace::USDAttribute &a) { return std::visit([](const auto &v)->py::object { return py::cast(v); }, a.get()); })
      .def("set", [](zSpace::USDAttribute &a, py::object value) {
        py::dict input; input["type"] = a.getTypeName(); input["value"] = value;
        a.set(usdValueFromPython(input).value);
      });
  py::class_<zSpace::USDPrim>(m, "USDPrim")
      .def("isValid", &zSpace::USDPrim::isValid)
      .def("getPath", &zSpace::USDPrim::getPath)
      .def("getTypeName", &zSpace::USDPrim::getTypeName)
      .def("getAttribute", &zSpace::USDPrim::getAttribute)
      .def("createAttribute", &zSpace::USDPrim::createAttribute, py::arg("name"), py::arg("type"), py::arg("custom") = true);
  py::class_<zSpace::USDdoc>(m, "USDdoc")
      .def(py::init<>())
      .def_static("supported", &zSpace::USDdoc::supported)
      .def("getPrimAtPath", &zSpace::USDdoc::getPrimAtPath)
      .def("definePrim", &zSpace::USDdoc::definePrim)
      .def("removePrim", &zSpace::USDdoc::removePrim)
      .def("setCustomLayerData", [](zSpace::USDdoc &d, py::dict values) {
        std::map<std::string, zSpace::zUSDValue> data;
        for (auto item : values) data[py::cast<std::string>(item.first)] = usdValueFromPython(py::cast<py::dict>(item.second));
        d.setCustomLayerData(data);
      })
      .def("readWorldPoints", &zSpace::USDdoc::readWorldPoints)
      .def("open", &zSpace::USDdoc::open)
      .def("close", &zSpace::USDdoc::close)
      .def("readMetadata",
           [](const zSpace::USDdoc &d) {
             const auto m = d.readMetadata();
             py::dict out;
             out["metersPerUnit"] = m.metersPerUnit;
             out["upAxis"] = m.upAxis;
             py::list prims;
             for (const auto &p : m.prims)
               prims.append(usdPrim(p));
             out["prims"] = prims;
             return out;
           })
      .def("readPrim",
           [](const zSpace::USDdoc &d, const std::string &p) {
             return usdPrim(d.readPrim(p));
           })
      .def("readAttribute",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) {
             auto a = d.readAttribute(p, n);
             auto out = usdValue(a);
             out["array"] = a.value.index() >= 6;
             out["interpolation"] = a.interpolation;
             out["custom"] = a.custom;
             return out;
           })
      .def("readCustomLayerData",
           [](const zSpace::USDdoc &d) {
             py::dict out;
             for (const auto &a : d.readCustomLayerData())
               out[py::str(a.first)] = usdValue(a.second);
             return out;
           })
      .def("hasAttribute", &zSpace::USDdoc::hasAttribute)
      .def("readBool", &zSpace::USDdoc::readBool)
      .def("tryReadBool",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             bool value{};
             if (!d.tryReadBool(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeBool",
          [](py::dict prim, const std::string &name, const bool &value,
             const std::string &interpolation, bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeBool(edit, name, value, interpolation, custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readInt", &zSpace::USDdoc::readInt)
      .def("tryReadInt",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             int value{};
             if (!d.tryReadInt(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeInt",
          [](py::dict prim, const std::string &name, const int &value,
             const std::string &interpolation, bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeInt(edit, name, value, interpolation, custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readFloat", &zSpace::USDdoc::readFloat)
      .def("tryReadFloat",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             float value{};
             if (!d.tryReadFloat(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeFloat",
          [](py::dict prim, const std::string &name, const float &value,
             const std::string &interpolation, bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeFloat(edit, name, value, interpolation,
                                       custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readDouble", &zSpace::USDdoc::readDouble)
      .def("tryReadDouble",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             double value{};
             if (!d.tryReadDouble(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeDouble",
          [](py::dict prim, const std::string &name, const double &value,
             const std::string &interpolation, bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeDouble(edit, name, value, interpolation,
                                        custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readString", &zSpace::USDdoc::readString)
      .def("tryReadString",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             std::string value{};
             if (!d.tryReadString(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeString",
          [](py::dict prim, const std::string &name, const std::string &value,
             const std::string &interpolation, bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeString(edit, name, value, interpolation,
                                        custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readIntArray", &zSpace::USDdoc::readIntArray)
      .def("tryReadIntArray",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             std::vector<int> value{};
             if (!d.tryReadIntArray(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeIntArray",
          [](py::dict prim, const std::string &name,
             const std::vector<int> &value, const std::string &interpolation,
             bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeIntArray(edit, name, value, interpolation,
                                          custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readFloatArray", &zSpace::USDdoc::readFloatArray)
      .def("tryReadFloatArray",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             std::vector<float> value{};
             if (!d.tryReadFloatArray(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeFloatArray",
          [](py::dict prim, const std::string &name,
             const std::vector<float> &value, const std::string &interpolation,
             bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeFloatArray(edit, name, value, interpolation,
                                            custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def_static(
          "writePoints",
          [](py::dict prim, const std::string &name,
             const std::vector<float> &value, const std::string &interpolation,
             bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writePoints(edit, name, value, interpolation,
                                        custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def_static(
          "writeNormals",
          [](py::dict prim, const std::string &name,
             const std::vector<float> &value, const std::string &interpolation,
             bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeNormals(edit, name, value, interpolation,
                                         custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readDoubleArray", &zSpace::USDdoc::readDoubleArray)
      .def("tryReadDoubleArray",
           [](const zSpace::USDdoc &d, const std::string &p,
              const std::string &n) -> py::object {
             std::vector<double> value{};
             if (!d.tryReadDoubleArray(p, n, value))
               return py::none();
             return py::cast(value);
           })
      .def_static(
          "writeDoubleArray",
          [](py::dict prim, const std::string &name,
             const std::vector<double> &value, const std::string &interpolation,
             bool custom) {
            zSpace::zUSDPrimEdit edit;
            zSpace::USDdoc::writeDoubleArray(edit, name, value, interpolation,
                                             custom);
            const auto &a = edit.attributes.at(name);
            auto out = usdValue(a);
            out["interpolation"] = a.interpolation;
            out["custom"] = a.custom;
            if (!prim.contains("attributes"))
              prim["attributes"] = py::dict();
            py::dict attributes = prim["attributes"].cast<py::dict>();
            attributes[py::str(name)] = out;
          },
          py::arg("prim"), py::arg("name"), py::arg("value"),
          py::arg("interpolation") = "", py::arg("custom") = true)
      .def("readPoints", &zSpace::USDdoc::readPoints)
      .def("applyEdits", [](zSpace::USDdoc &d,
                            py::dict edits) { d.applyEdits(usdEdits(edits)); })
      .def("save", &zSpace::USDdoc::save)
      .def("text", &zSpace::USDdoc::text);
#endif
}
