# Extending zspace_core

This guide is for humans and Codex/LLM agents adding new code to
`zspace_core`. Read this before creating new classes, moving code, or changing
the build.

## Repository Purpose

`zspace_core` is a C++17 geometry framework split into focused DLLs:

- `zSpace_Core`: low-level math, geometry storage, fields, dynamics, utilities.
- `zSpace_Interface`: public geometry objects, iterators, and function sets.
- `zSpace_IO`: file IO facade and format codecs.
- `zSpace_Display`: optional display settings, scenes, and OpenGL backend.
- `zSpace_InterOp`: optional host/SDK interop, currently Rhino/OpenNURBS based.

The public API direction is object + function set:

```cpp
zObjectMesh mesh;
zFnMesh fn(mesh);
```

Objects own identity and storage. Function sets create, edit, and query
geometry. Display classes own rendering state. IO classes read and write
external formats.

## Active Folder Structure

```text
include/zspace/                 Public umbrella headers and public API.
include/zspace/zCore/           Public core math, geometry, field, and utility headers.
include/zspace/zInterface/      Public objects, iterators, and function sets.
include/zspace/zIO/             Public IO facade, codecs, and result types.
include/zspace/zDisplay/        Public display scene, display settings, render context.
include/zspace/zInterOp/        Public optional host/SDK interop headers.

src/zCore/                      Core implementation files.
src/zInterface/objects/         Object implementation and private object storage.
src/zInterface/functionsets/    Function-set implementation.
src/zInterface/iterators/       Iterator implementation.
src/zIO/                        IO facade and codec implementation.
src/zDisplay/                   Display and renderer implementation.
src/zInterOp/                   Optional Rhino/Maya/Unreal/USD interop implementation.

agents/                         Agent-facing extension and usage guides.
scripts/                        Supported build scripts.
tests/smoke/                    Smoke tests for build and basic API health.
tests/headers/                  Header compile checks.
docs/architecture/              Architecture and migration notes.
legacy/                         Archived legacy material.
archive/                        Archived or removed dependencies/features.
third_party/depends/            Vendored third-party dependencies still used.
build/                          Generated build output, ignored by git.
```

Do not add new active code to `legacy/`, `archive/`, or `build/`.

## Build Setup

Preferred build path: CMake + Ninja + MSVC from VS Code or the terminal.

Requirements:

- Visual Studio Build Tools or Visual Studio 2022 with C++ desktop tools.
- CMake 3.21 or newer.
- Ninja. Visual Studio's bundled Ninja is acceptable.

Default PowerShell build:

```powershell
.\scripts\build.ps1 -Configuration Release
```

Default Command Prompt build:

```bat
scripts\build_core_interface.bat
```

Raw CMake commands from a Visual Studio Developer shell:

```powershell
cmake --preset ninja-msvc
cmake --build --preset ninja-msvc-release --parallel
```

Run tests:

```powershell
ctest --test-dir build\ninja-msvc -C Release --output-on-failure
```

Default DLL outputs:

```text
build/ninja-msvc/bin/zSpace_Core.dll
build/ninja-msvc/bin/zSpace_Interface.dll
build/ninja-msvc/bin/zSpace_IO.dll
build/ninja-msvc/bin/zSpace_Display.dll
```

Import libraries are written to:

```text
build/ninja-msvc/lib/
```

## CMake Target Rules

The dependency direction is:

```text
zSpace_IO -> zSpace_Interface -> zSpace_Core
zSpace_Display -> zSpace_Interface -> zSpace_Core
zSpace_InterOp -> zSpace_IO or zSpace_Interface
```

Rules:

- `zSpace_Core` must not depend on `zSpace_Interface`, `zSpace_IO`,
  `zSpace_Display`, or host SDKs.
- `zSpace_Interface` may depend on `zSpace_Core`.
- `zSpace_IO` may depend on `zSpace_Interface`.
- `zSpace_Display` may depend on `zSpace_Interface`.
- Host SDK code belongs in `zSpace_InterOp`, not core/interface/display.
- OpenGL code belongs in `zSpace_Display`, not `zSpace_Core` or
  `zSpace_Interface`.

The root `CMakeLists.txt` uses `GLOB_RECURSE` with `CONFIGURE_DEPENDS`.
When adding a new `.cpp` under an existing source tree, it should be picked up
automatically on the next configure/build.

## Naming Conventions

Use keyword-first public names:

```text
zObject<Type>       Geometry/data object with identity and private storage.
zFn<Type>           Function set for creating, editing, and querying an object.
zIt<Type><Part>     Iterator over components of an object.
zDisplay<Type>      Display settings and draw behavior for an object type.
zCodec<Format>      Format-specific IO codec.
zIO                 Stateless facade for common read/write operations.
```

Examples:

```text
zObjectMesh
zFnMesh
zItMeshVertex
zDisplayMesh
zCodecOBJ
```

Compatibility headers named `zObj*` may remain, but new code should not add new
`zObj*` API names. If compatibility is required, add only an alias header that
maps `zObj<Type>` to `zObject<Type>`.

## Public API Rules

Objects:

- Own object identity, transform, and storage.
- Should not expose raw storage as public members.
- Should not contain draw methods.
- Should not contain file IO methods.
- Should remain lightweight from a user perspective.

Function sets:

- Own create/edit/query operations.
- Attach to objects by reference or explicit attach methods.
- Should be the main way users modify geometry.
- Should expose ergonomic methods and usage examples in comments where useful.

Iterators:

- Traverse object components.
- May use private object storage through friendship or private accessors.
- Should avoid exposing STL iterators directly as public API.

Display:

- Owns rendering state and draw behavior.
- Belongs under `zDisplay`, not `zInterface` objects.
- Should support drawing multiple scene objects through reusable display settings.

IO:

- Belongs under `zIO`.
- Public users should prefer `zIO::readMesh`, `zIO::writeMesh`,
  `zIO::readGraph`, `zIO::writeGraph`, and similar facade methods.
- Format details belong in `zCodec<Format>` classes.

Interop:

- Belongs under `zInterOp`.
- Any dependency on Rhino, Maya, Unreal, USD, or another SDK must be optional and
  guarded by CMake options.

## Adding A New Object Type

For a new geometry type named `Foo`, prefer this structure:

```text
include/zspace/zInterface/objects/zObjectFoo.h
include/zspace/zInterface/functionsets/zFnFoo.h
include/zspace/zInterface/iterators/zItFoo.h       optional
src/zInterface/objects/zObjectFoo.cpp
src/zInterface/functionsets/zFnFoo.cpp
src/zInterface/iterators/zItFoo.cpp                optional
```

If the object needs private storage, add it under:

```text
src/zInterface/objects/internal/zFooStorage.h
src/zInterface/objects/internal/zFooStorage.cpp
```

or use an object-specific impl type:

```text
src/zInterface/objects/internal/zObjectFooImpl.h
```

Basic public object pattern:

```cpp
class ZSPACE_API zObjectFoo : public zObject
{
public:
    zObjectFoo();
    ~zObjectFoo();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;

    friend class zFnFoo;
};
```

Use a simpler private storage member only when templates or existing patterns
make full PIMPL awkward. Keep the member private and expose it only to the
owning `zFn*` and `zIt*` classes.

## Adding A New Function Set

Function sets should attach to one object type and expose user-facing operations.

Header:

```cpp
class ZSPACE_API zFnFoo : public zFn
{
protected:
    zObjectFoo* fooObj = nullptr;

public:
    zFnFoo();
    zFnFoo(zObjectFoo& fooObj);

    void create(...);
    void clear();
    int numElements() const;
};
```

Implementation:

```cpp
ZSPACE_INLINE zFnFoo::zFnFoo(zObjectFoo& _fooObj)
{
    fooObj = &_fooObj;
    fnType = zFnType::zInvalidFn; // Add a specific enum when needed.
}
```

Rules:

- Prefer `zObjectFoo&`, not `zObjFoo&`.
- Do not put format-specific read/write logic in `zFnFoo`.
- Do not put OpenGL drawing in `zFnFoo`.
- Keep methods small and query/edit focused.

## Adding Display Support

Add display support only under `zDisplay`:

```text
include/zspace/zDisplay/display/zDisplayFoo.h
src/zDisplay/display/zDisplayFoo.cpp
```

If rendering logic becomes large, keep render-specific code in:

```text
src/zDisplay/display/zDisplayFooRender.cpp
```

Usage should look like:

```cpp
zObjectFoo foo;
zDisplayScene scene;

zDisplayFoo& display = scene.foo(foo);
display.setVisible(true);

scene.draw(foo);
```

Avoid adding draw methods back to `zObjectFoo`.

## Adding IO Support

Add format-specific codecs under `zIO`:

```text
include/zspace/zIO/codecs/zCodecFoo.h
src/zIO/codecs/zCodecFoo.cpp
```

Then expose simple operations through `zIO`:

```cpp
zIO::readMesh("mesh.foo", mesh);
zIO::writeMesh("mesh.foo", mesh);
```

Rules:

- `zIO` is the user-facing facade.
- `zCodec<Format>` is the implementation boundary.
- Do not add `fromJSON`, `toJSON`, `fromOBJ`, or `toOBJ` methods to function
  sets.
- Format selection should normally be extension-based.

## Adding Interop Support

Interop code belongs under:

```text
include/zspace/zInterOp/
src/zInterOp/
```

The default build keeps interop off. Interop should be enabled with explicit
CMake options:

```text
ZSPACE_BUILD_INTEROP=ON
ZSPACE_WITH_RHINO=ON
ZSPACE_RHINO_DIR=...
```

Rhino/OpenNURBS expected layout:

```text
C:/Program Files/Rhino 8 SDK/inc
C:/Program Files/Rhino 8 SDK/lib/Release
```

Build:

```bat
scripts\build_interop.bat "C:\Program Files\Rhino 8 SDK"
```

Do not make core/interface/display depend on a host SDK.

## Agent Workflow For Changes

Use this checklist when adding or modifying API:

1. Read this file and `docs/architecture/api-migration.md`.
2. Search for the nearest existing type and follow its pattern.
3. Add public headers under `include/zspace/...`.
4. Add implementations under `src/...`.
5. Keep dependencies flowing in the approved direction.
6. Use `zObject*`, `zFn*`, `zIt*`, `zDisplay*`, `zCodec*` names.
7. Keep storage private or behind an internal implementation.
8. Keep IO out of function sets.
9. Keep drawing out of objects.
10. Build Release.
11. Run smoke tests.
12. Update docs when architecture or usage changes.

Build verification:

```powershell
.\scripts\build.ps1 -Configuration Release
ctest --test-dir build\ninja-msvc -C Release --output-on-failure
```

## Current Migration State

Completed:

- CMake + Ninja build path for CLI and VS Code.
- DLL build targets for Core, Interface, IO, and Display.
- Legacy Visual Studio project files moved out of the active build path.
- Display separated from geometry objects.
- Mesh and graph object storage moved behind opaque/internal storage.
- Point-cloud storage moved behind an internal implementation.
- Public iterators migrated to `zObject*` handles.
- Public function-set signatures migrated to `zObject*` names for mesh, graph,
  point cloud, mesh field, point field, and dynamics APIs.
- Field and particle storage members made private with targeted internal access.
- `zIO` facade introduced for mesh/graph/point and codec-based IO.
- libigl dependency removed from mesh curvature.
- Bundled dependency include paths marked as CMake `SYSTEM` includes.
- Field STL storage/iterator DLL-interface warnings suppressed at private
  storage members.

Still in progress:

- Reduce public header dependency graph further.
- Move template field storage behind a cleaner private/internal boundary if a
  non-template facade is introduced.
- Keep `zObj*` compatibility aliases until downstream code has migrated.
- Review optional interop boundaries so host SDK types do not leak into the
  default geometry API.

## Compatibility Policy

During the migration, avoid breaking existing users unnecessarily:

- Keep existing method names when behavior remains the same.
- Prefer adding `zObject*` names while leaving `zObj*` aliases as bridges.
- Do not remove compatibility aliases without a dedicated deprecation pass.
- Document any intentional breaking change in this file and in
  `docs/architecture/api-migration.md`.

## What Not To Do

Do not:

- Add new active Visual Studio `.sln` or `.vcxproj` build paths.
- Put generated build files in git.
- Add new public `zObj*` classes.
- Add draw methods to `zObject*`.
- Add file-format methods to `zFn*`.
- Add host SDK dependencies to Core, Interface, IO, or Display.
- Expose raw storage as public object members.
- Reintroduce libigl for curvature without a clear dependency plan.
