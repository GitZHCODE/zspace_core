# zSpace Core

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![GitHub release](https://img.shields.io/github/v/release/GitZHACODE/zspace_core)](https://github.com/GitZHACODE/zspace_core/releases)
[![GitHub issues](https://img.shields.io/github/issues/GitZHACODE/zspace_core)](https://github.com/GitZHACODE/zspace_core/issues)

zSpace is a modular C++17 library for geometry data structures and algorithms. It uses CMake and can be built as dynamically linked libraries by default, or as static libraries when required. The library is divided into Core, Interface, IO, Display, and optional InterOp modules, allowing applications to link only the functionality they need. A header-only compatibility mode remains available for existing integrations but is not the recommended build configuration.

## Contents

- [Build](#build)
- [Geometry IO](#geometry-io)
- [Optional InterOp](#optional-interop)
- [Repository layout](#repository-layout)
- [Citation](#citation)
- [Dependencies](#dependencies)
- [License](#license)

## Build

The preferred build path is CMake with Ninja. It builds separate Core, Interface, IO, and Display libraries while keeping Maya, Rhino, Unreal, and USD/Omniverse interoperability opt-in. This workflow does not generate Visual Studio `.sln` or `.vcxproj` files.

The dependency direction is:

```text
zSpace_IO      -> zSpace_Interface -> zSpace_Core
zSpace_Display -> zSpace_Interface -> zSpace_Core
zSpace_InterOp -> zSpace_IO        -> zSpace_Interface -> zSpace_Core
```

`zSpace_Core` and `zSpace_Interface` contain no OpenGL dependency. Headless geometry applications only need those two DLLs. `zSpace_Display` is optional and owns the OpenGL, GLEW, and FreeGLUT rendering backend.

Mesh geometry and display settings are configured separately:

```cpp
zObjectMesh mesh;
zDisplayScene scene;

zDisplayMesh& display = scene.mesh(mesh);
display.setDisplayElements(false, true, true);
display.setDisplayVertexNormals(true, 0.5);

scene.draw(mesh);
```

`zObjectMesh` owns geometry. `zDisplayMesh` owns visibility and drawing settings.

Geometry queries and edits belong to function sets:

```cpp
zFnMesh fnMesh(mesh);

zPoint minBounds;
zPoint maxBounds;
fnMesh.getBounds(minBounds, maxBounds);
```

Geometry objects no longer expose `getBounds()` directly.

### Requirements

- MSVC Build Tools 2022 with the C++ compiler and Windows SDK. The Visual Studio IDE is not required.
- CMake and Ninja on `PATH`.

From PowerShell:

```powershell
.\scripts\build.ps1 -Configuration Release
```

Or from Command Prompt:

```bat
scripts\build_core_interface.bat
```

Equivalent raw CMake commands, when run from a Visual Studio Developer PowerShell or Developer Command Prompt:

```powershell
cmake --preset ninja-msvc
cmake --build --preset ninja-msvc-release --parallel
```

### Build outputs

The default build produces:

- `zSpace_Core.dll`
- `zSpace_Interface.dll`
- `zSpace_IO.dll`
- `zSpace_Display.dll`

## Geometry IO

File IO is provided by the separate `zSpace_IO` library through one stateless facade:

```cpp
#include <zspace/io.h>

zSpace::zObjectMesh mesh;
auto result = zSpace::zIO::readMesh("mesh.obj", mesh);

if (!result)
    std::cerr << result.message();
```

The same API supports graphs and writing:

```cpp
zSpace::zIO::writeMesh("mesh.json", mesh);
zSpace::zIO::readGraph("graph.txt", graph);
zSpace::zIO::writeGraph("graph.json", graph);
```

Formats are selected from the file extension. OBJ, JSON, and graph TXT codecs are
built into `zSpace_IO`. TinyUSDZ provides mesh reading for USDA, USDC, and USDZ,
plus robust USDA writing. Binary USDC and USDZ writing is intentionally disabled
until TinyUSDZ's writers are production-ready. Configure with
`ZSPACE_IO_WITH_TINYUSDZ=OFF` to build without USD support.

USD mesh import currently reads the first mesh prim in object space. Scene
composition, transforms, animation, and multi-mesh stage import remain outside
the single-object `zIO::readMesh` contract.

The generated binaries and import libraries are placed under the selected CMake preset directory:

```text
build/ninja-msvc/bin/zSpace_Core.dll
build/ninja-msvc/bin/zSpace_Interface.dll
build/ninja-msvc/bin/zSpace_IO.dll
build/ninja-msvc/bin/zSpace_Display.dll
build/ninja-msvc/lib/zSpace_Core.lib
build/ninja-msvc/lib/zSpace_Interface.lib
build/ninja-msvc/lib/zSpace_IO.lib
build/ninja-msvc/lib/zSpace_Display.lib
```

In VS Code, open the repository folder and run the default build task: `CMake: build Release DLLs`.

## Optional InterOp

`zSpace_InterOp` is included in the CMake structure as an opt-in target. The current interop API uses Rhino/OpenNURBS types directly for plane, arc, curve, and nurbs objects, so it cannot be built as an SDK-free DLL yet.

To enable it, configure with the Rhino SDK path:

```bat
scripts\build_interop.bat "C:\Path\To\Rhino\SDK"
```

This path should be the SDK root that contains:

```text
inc/
lib/Release/
```

Expected Rhino SDK paths:

```text
%RhinoDir%/inc
%RhinoDir%/lib/Release
opennurbs.lib
RhinoCore.lib
RhinoLibrary.lib
```

You can also set `RhinoDir` once and run the batch without arguments:

```bat
set "RhinoDir=C:\Path\To\Rhino\SDK"
scripts\build_interop.bat
```

Equivalent raw CMake commands:

```powershell
cmake --preset ninja-msvc `
  -DZSPACE_BUILD_INTEROP=ON `
  -DZSPACE_WITH_RHINO=ON `
  -DZSPACE_RHINO_DIR="C:/Path/To/Rhino/SDK"

cmake --build --preset ninja-msvc-release --parallel
```

Additional integrations are exposed as explicit switches:

```text
ZSPACE_WITH_MAYA=ON    requires ZSPACE_MAYA_DIR
ZSPACE_WITH_UNREAL=ON  requires ZSPACE_UNREAL_DIR
ZSPACE_WITH_USD=ON     requires ZSPACE_USD_DIR
```

Large host SDKs are not bundled with this repository. Set the corresponding
`ZSPACE_*_DIR` option to an external installation when enabling an integration.

The default CLI and VS Code build keeps interop disabled so `zSpace_Core` and `zSpace_Interface` remain fast and reproducible on a standard C++ toolchain.

## Repository layout

The active build uses this folder structure:

```text
include/zspace/        Public headers for zCore, zInterface, zDisplay, and zInterOp.
src/zCore/             Geometry data structures and foundational utilities.
src/zInterface/        Geometry objects, iterators, and function sets.
src/zDisplay/          Optional scene, display settings, model, and OpenGL backend.
src/zInterOp/          Optional host and SDK interoperability.
third_party/depends/   Vendored third-party dependencies used by the current code.
scripts/               Active developer scripts, including the CMake + Ninja build wrapper.
legacy/                Archived Visual Studio solution/projects and old deploy batch scripts.
build/                 Generated CMake/Ninja build output. This folder is ignored by git.
```

`legacy/visualstudio` is kept for reference only. The maintained build entry point is the root `CMakeLists.txt`.

## Citation

When using zSpace in academic or published work, cite the repository:

```bibtex
@software{ZSPACE,
  title  = {ZSPACE: A C++ Geometry Data Structures and Algorithms Library},
  author = {Bhooshan, Vishu and Bhooshan, Shajay and Chen, Taizhong and others},
  year   = {2018},
  url    = {https://github.com/GitZHACODE/zspace_core}
}
```

## Dependencies

Dependencies used by the default Core, Interface, and IO build are either
vendored under `third_party/depends` or fetched at a pinned revision by CMake.

| Dependency | Purpose | License |
| --- | --- | --- |
| [Eigen](https://eigen.tuxfamily.org/) | Dense and sparse linear algebra | MPL-2.0 |
| [JSON for Modern C++](https://github.com/nlohmann/json) | JSON parsing and serialization | MIT |
| [QuickHull](https://github.com/karimnaaji/quickhull) | Convex hull computation | MIT |
| [LodePNG](https://lodev.org/lodepng/) | PNG encoding | Zlib |
| [TooJPEG](https://create.stephan-brumme.com/toojpeg/) | JPEG encoding | Zlib |
| [TinyUSDZ](https://github.com/lighttransport/tinyusdz) | USDA/USDC/USDZ mesh IO | Apache-2.0 |

The optional Display module adds the following graphics dependencies:

| Dependency | Purpose | License |
| --- | --- | --- |
| [OpenGL](https://www.khronos.org/opengl/) | Rendering API | Platform/system library |
| [GLEW](https://glew.sourceforge.net/) | OpenGL extension loading | Modified BSD, MIT |
| [FreeGLUT](https://freeglut.sourceforge.net/) | OpenGL windowing and utilities | MIT/X11 |

The optional InterOp module links against external SDKs selected at configure time. Rhino/OpenNURBS is currently required to build InterOp; Maya, Unreal Engine, and USD/Omniverse support are optional. These SDKs are not vendored, and their licenses and redistribution terms are governed by their respective providers.

Armadillo support remains available behind the `USING_ARMA` compile definition. Spectra and ALGLIB are present in the vendored dependency tree for historical compatibility but are not used by the maintained CMake targets.

## License

zSpace Core is licensed under the [MIT License](LICENSE). Third-party components retain their respective licenses.
