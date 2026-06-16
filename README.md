# ZSPACE
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/gitzhcode/zspace_core/LICENSE.MIT)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://github.com/gitzhcode/zspace_core/doxyoutput/)
[![GitHub Releases](https://img.shields.io/github/release/gitzhcode/zspace_core.svg)](https://github.com/gitzhcode/zspace_core/releases)
[![GitHub Issues](https://img.shields.io/github/issues/gitzhcode/zspace_core.svg)](http://github.com/gitzhcode/zspace_core/issues)

**ZSPACE** is a C++  library collection of geometry data-structures, algorithms framework. It is implemented as a header-only C++ library, whose dependencies, are header-only or static libraries. Hence **ZSPACE** can be easily embedded in C++ projects. 

Optionally the library may also be pre-compiled into a statically  or dynamically linked library, for faster compile times.

- [Citing](#Citing)
- [License](#license)
- [Third party dependcencies](#used-third-party-dependencies)

# Build from CLI or VS Code

The preferred build path is CMake + Ninja. It builds separate Core, Interface, and Display DLLs, avoids the optional Maya/Rhino/Unreal/Omniverse interop projects, does not generate Visual Studio `.sln` or `.vcxproj` files, and emits DLLs to the repository `build` folder.

The dependency direction is:

```text
zSpace_Display -> zSpace_Interface -> zSpace_Core
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

Requirements:

- Visual Studio 2022 with the C++ desktop workload.
- CMake and Ninja on `PATH`, or Visual Studio's bundled CMake and Ninja.

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
built into `zSpace_IO`. USD extensions use the `zCodecUSD` boundary and report
that the optional USD module is unavailable when it is not enabled.

Build outputs:

```text
build/ninja-msvc/bin/zSpace_Core.dll
build/ninja-msvc/bin/zSpace_Interface.dll
build/ninja-msvc/bin/zSpace_Display.dll
build/ninja-msvc/lib/zSpace_Core.lib
build/ninja-msvc/lib/zSpace_Interface.lib
build/ninja-msvc/lib/zSpace_Display.lib
```

In VS Code, open the repository folder and run the default build task: `CMake: build Release DLLs`.

## Optional interop build

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

The default CLI and VS Code build keeps interop disabled so `zSpace_Core` and `zSpace_Interface` remain fast and reproducible on a standard C++ toolchain.

# Repository layout

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

# Citing
If you use the library of ZSPACE in a project, please refer to the GitHub repository. <br/> <br/>
@misc{zspace-framework, <br/>
      title  = {{zspace}: A simple C++ header-only collection of geometry data-structures, algorithms and city data visualization                       framework.},<br/>
      author = {Vishu Bhooshan, Shajay Bhooshan, Tommaso Casucci, Taizhong Chen and others},<br/>
      note   = {https://github.com/venumb/ZSPACE},<br/>
      year   = {2018},<br/>
    }

# License
The library is licensed under the [MIT License](https://opensource.org/licenses/MIT).


# Third party dependencies
The library has some dependencies on third-party tools and services, which have different licensing as listed below.
Thanks a lot!

- [**OPENGL**](https://www.opengl.org/about/) for display methods. End users, independent software vendors, and others writing code based on the OpenGL API are free from licensing requirements.

- [**Eigen**](https://github.com/eigenteam/eigen-git-mirror) for matricies and related methods. It is an open source project licensed under
[MPL2](https://www.mozilla.org/MPL/2.0/).

- [**Spectra**](https://github.com/yixuan/spectra) for large scale eigen value problems. It is an open source project licensed under
[MPL2](https://www.mozilla.org/MPL/2.0/).

- [**Armadillo**](http://arma.sourceforge.net/) for matricies and related methods. It is an open source project licensed under
[Apache License 2.0](https://opensource.org/licenses/Apache-2.0).

- [**Alglib**](http://http://www.alglib.net/) free edition for linear programming optimisation methods.

- [**JSON for Modern C++**](https://github.com/nlohmann/json) to create a JSON file. It is an open source project licensed under
[MIT License](https://opensource.org/licenses/MIT).

- [**SQLITE**](https://www.sqlite.org/index.html) for SQL database engine. It is an open source project dedicated to the [public domain](https://en.wikipedia.org/wiki/Public_domain).

- [**LodePNG**](https://lodev.org/lodepng) for creating PNG images. It is a project licensed under 
[ZLIB License](https://zlib.net/zlib_license.html).

- [**TooJPEG**](https://create.stephan-brumme.com/toojpeg/) for creating JPEG images. It is a project licensed under 
[ZLIB License](https://zlib.net/zlib_license.html).
