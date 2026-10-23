#pragma once

#include <pybind11/pybind11.h>

namespace zspace_python {

// Each function registers one group of bindings into the `_core` module.
// Split across files purely for readability - order matters because later
// groups (zFnMesh, zFnGraph) reference types registered by earlier ones
// (zVector, zColor, zObjMesh, zObjGraph).
void bind_core_types(pybind11::module_ &m);  // zVector, zColor, shared enums
void bind_obj_mesh(pybind11::module_ &m);    // zObjMesh
void bind_fn_mesh(pybind11::module_ &m);     // zFnMesh
void bind_obj_graph(pybind11::module_ &m);   // zObjGraph
void bind_fn_graph(pybind11::module_ &m);    // zFnGraph

}  // namespace zspace_python
