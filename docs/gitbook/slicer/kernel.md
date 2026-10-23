# Kernel foundations for slicer

zSpace Core supplies the C++17 geometry infrastructure used by the slicer engine.
It does not own the slicing algorithms or application controls.

## Geometry and ownership

Geometry objects own data. Function sets create, query and modify that data;
iterators traverse geometry components. The public naming convention uses
`zObjectMesh`, `zObjectGraph`, `zFnMesh` and `zFnGraph`. Existing toolset APIs
also use the compatible `zObjMesh` and `zObjGraph` names.

Slicing uses mesh geometry for the input and sections, scalar fields for path
construction, and graphs for bracing and print paths. Toolsets owns its slicer
sessions and derived results. Applications own their controls and display state.

Core's module dependencies are:

```text
zSpace_Interface -> zSpace_Core
zSpace_IO        -> zSpace_Interface -> zSpace_Core
zSpace_Display   -> zSpace_Interface -> zSpace_Core
```

Core and Interface have no OpenGL dependency. Native applications choose their
display integration; browser applications copy geometry across a WASM boundary
into viewer-owned buffers.

## Mesh IO versus slicer input

`zIO::readMesh` and `zIO::writeMesh` are generic geometry operations. Optional
OpenUSD support is selected by `ZSPACE_IO_WITH_OPENUSD` with an external SDK.
The generic USD mesh reader reads the first composed mesh in object space; it
does not bake transforms or convert units/up-axis.

Toolsets' `zTsIO::readSlicerInputUSD` is a separate adapter for the tagged slicer
schema. Its current native contract includes the block, bracing, slot point,
settings and tag tables, and normalizes dimensional data to millimetres. Do not
replace this adapter with a generic mesh read when slicer metadata is needed.

Native OpenUSD libraries cannot be used in a browser. Browser USD functionality
needs an Emscripten build of its SDK and the appropriate importer module.

## Building the kernel

For the maintained Windows native path, install MSVC Build Tools 2022 with the
C++ compiler and Windows SDK, plus CMake and Ninja. From the core repository:

```powershell
.\scripts\build.ps1 -Configuration Release
```

Toolsets and Alice 2 can instead include the sibling core source through their
own CMake builds. Follow those guides for their dependencies and path overrides.
The standard core WASM build and the dedicated toolsets slicing WASM build are
separate targets; rebuilding core alone does not rebuild the slicer.

## Sources

- [Core README](https://github.com/GitZHACODE/zspace_core/blob/772c42caba7315b7d921ba4c96d38e1962b67471/README.md)
- [Public API naming and migration](https://github.com/GitZHACODE/zspace_core/blob/772c42caba7315b7d921ba4c96d38e1962b67471/docs/architecture/api-migration.md)
- [OpenUSD IO contracts](https://github.com/GitZHACODE/zspace_core/blob/772c42caba7315b7d921ba4c96d38e1962b67471/docs/architecture/openusd-io.md)
