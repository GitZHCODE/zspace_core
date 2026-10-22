# Mesh const-access implementation summary

Date: 2026-10-22. Task: TASK-005-mesh-const-access.
Repository: zspace_core, branch `zspace_v3`; baseline commit
`980d8bd10dd2fe984117d5850e1ab16bd31b0d8f`. Changes are local and uncommitted.

## Cause and resulting behavior

Mesh iterator and function-set queries passed a mutable mesh to
`zMeshObjectStorage::get()`. That overload marks the face-list dirty even when
the caller only reads. Subsequent topology queries synchronize the entire
face-list, including geometry and attributes. Repeated navigation could therefore
perform whole-mesh work instead of reusing the topology cache.

The audited query paths now explicitly select the const storage overload.
Existing iterator signatures are unchanged. A cold query can still construct
topology, and a query following a genuine edit can still synchronize storage.
Clean, unchanged read-only traversal does not itself mark the face-list dirty.

## Changes by file

| File | Change |
| --- | --- |
| [zItMesh.cpp](../../src/zInterface/iterators/zItMesh.cpp) | Const access for connected-half-edge queries, primitive adjacency, half-edge indexed construction, end/size, navigation, positions, boundary, color, active state, and dihedral reads. Vertex area uses a local utility. Center/vector arithmetic uses copied positions because legacy vector operators are non-const. |
| [zFnMesh.cpp](../../src/zInterface/functionsets/zFnMesh.cpp) | Const reads for half-edge count/lookup, dual source queries and vertex areas. Utility calls use a local `zUtilsCore`. Dual destination creation and rotation share one writable scope; subsequent destination queries use const access. |
| [zHEGeomTypes.h](../../include/zspace/zCore/geometry/zHEGeomTypes.h), [zHEGeomTypes.cpp](../../src/zCore/geometry/zHEGeomTypes.cpp) | Added nine const overloads: vertex/edge/face getHalfEdge and half-edge getSym/getPrev/getNext/getVertex/getFace/getEdge. Existing non-const symbols remain and forward to const implementations. These additional prerequisites were discovered by compilation. |
| [zHalfEdgeTopology.h](../../src/zCore/geometry/detail/zHalfEdgeTopology.h), [zHalfEdgeTopology.cpp](../../src/zCore/geometry/zHalfEdgeTopology.cpp) | Added const halfEdgeExists; lookup only reads the map and writes the caller's result ID. Retained the old non-const overload as a forwarding entry point. |
| [zMeshObjectStorage.h](../../src/zInterface/objects/zMeshObjectStorage.h) | Added internal, non-synchronizing needsFaceListSync diagnostic for regression checks. Existing get/read/edit dirty-flag behavior is unchanged. |
| [mesh_const_access_fixture.h](../../tests/smoke/mesh_const_access_fixture.h), [zspace_smoke.cpp](../../tests/smoke/zspace_smoke.cpp) | Added and registered cache and geometry regression checks in the existing native smoke suite. |

Actual setters, topology edits and writable raw accessors still use writable
storage. No broad removal of invalidation flags or const_cast workaround was used.
The public getter overload additions preserve ordinary existing calls and old
non-const symbols; code taking an unqualified address of an overloaded getter may
need an explicit member-function-pointer type. Rebuild consumers against the
updated Core/Interface pair.

## Verification

Native Release compilation succeeded with MSVC Build Tools 2022 (19.44).
The checked-in build script assumes Visual Studio Professional, which is not
installed here. An equivalent configure/build was run using the installed
Build Tools vcvars64 and Ninja paths; no build script or preset was changed.

```powershell
# Run from a Build Tools x64 developer environment, in zspace_core:
cmake --preset ninja-msvc "-DCMAKE_MAKE_PROGRAM=C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe"
cmake --build --preset ninja-msvc-release --parallel 6
ctest --test-dir build/ninja-msvc -C Release --output-on-failure
```

All five registered native tests passed: graph parity, graph retained views,
graph construction, general smoke and IO smoke. New smoke coverage includes:

- Cold topology initialization followed by repeated cache reuse.
- Boundary and interior half-edge navigation and opposite/next/previous identities.
- Successful and missing edge lookups, vertex/edge/face queries and clean flags.
- Unit-square area conservation and positive closed-mesh vertex areas.
- Tetrahedral dual mesh and graph output, with and without rotation, while the input stays clean.
- Genuine topology-position edits marking the face-list dirty and synchronizing correctly.
- Face-list position setters invalidating topology and rebuilding it with the new position.

Tests inspect dirty state and topology identity; no production synchronization
counters or timing instrumentation were added. Build/test logs are local ignored
files under `build/mesh-const-build.log` and `build/mesh-const-tests.log`.

## Limits and follow-up

- Alice2, Toolset, Python and WASM artifacts were not rebuilt or deployed.
  Existing application binaries do not acquire this fix until rebuilt/relinked.
- No TopologySlicer before/after path comparison or performance benchmark was run.
  The cache regression is verified; an application-level speedup is not measured.
- Const access does not make lazy cache initialization thread-safe. Warm caches
  before parallel reads and prohibit concurrent mutation or retained writable
  pointer edits during those reads.
- Existing USD-related changes in cmake/ZSpaceOpenUSD.cmake and wasm files were
  preserved and are not part of this task. Native verification used USD disabled.
- No commit or push was performed. Reviewer acceptance remains pending.
