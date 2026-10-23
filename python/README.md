# Core Python bindings

Core owns the pybind sources in `core/src`. Build Core independently:

```powershell
./build_headless.ps1 -PythonExecutable C:/Path/To/Python313/python.exe
```

Requires Visual Studio 2022, CMake 3.21+, x64 Python 3.10 or 3.13, and pybind11 installed for that interpreter (or `-Pybind11Dir`). Output: `build_headless313/package/zspace/`, with the ABI-tagged `_core.pyd` in `core/` and Core/Interface DLLs in `runtime/`. Add `build_headless313/package` to PYTHONPATH and use `import zspace.core`.

Rendering and OpenGL are disabled. USD is off by default; `-WithUSD` enables Core IO bindings and runtime deployment using Core SDK resolution. No Toolsets checkout is needed.

Toolsets builds include `HeadlessPython.cmake` and add `python/core` after defining the native Core targets. They do not invoke this standalone entry point. Both builds use `ZSPACE_PYTHON_PACKAGE_DIR` for package output. Use separate build directories for standalone and combined builds; recreate caches from before this relocation. Native algorithms and public imports are unchanged.

## NumPy batch geometry

`zFnMesh.create_numpy(positions, face_counts, face_indices)` and `zFnGraph.create_numpy(positions, edge_indices, precision=PRECISION)` accept `(N,3)` numeric positions and one-dimensional integer topology arrays (edges are flattened pairs). Non-contiguous coordinates are normalized to contiguous storage; invalid shapes, coordinates and connectivity raise ValueError before mutation. Coordinates are stored at Core float precision.

Both function sets provide `get_positions_numpy()` and `set_positions_numpy(positions)`. `get_mesh_data_numpy()` returns positions/counts/indices; `get_graph_data_numpy()` returns positions/edges. Outputs are NumPy-owned independent copies, not live views. Existing list APIs remain supported.

USD-enabled builds place USD DLLs and resources in the shared `runtime/` directory (`usd/` and `plugin/`). The root package registers this DLL directory before importing Core. Legacy per-module copies are removed after deployment.
