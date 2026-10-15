# zSpace Core WASM

This folder owns the browser/WASM build for `zspace_core`.

## Planarisation (Mode 3)

Mode 3 combines enabled `zFnMeshDynamics` face-planarity, vertex-group-plane and
rigid-line forces. Default mode 3 configuration enables only face planarity.
`zspace_planarity_params(mask, quad, strength, tolerance, groupStrength,
groupTolerance, pairStrength, pairTolerance)` selects bits 1/2/4 and the method.
`zspace_planarity_constraints(jsonOrAbsoluteFsPath)` accepts `{groups, pairs}`:
groups contain `vertices`, 3D `origin`, unit `normal`; pairs are `[a,b,length]`.
Use a WASM FS JSON file for large constraint collections: passing the entire
document through ccall's string stack can overflow. Vertex IDs are validated.
`zspace_planarity_deviation(kind)` returns current maximum absolute deviation
for face/group/pair kinds 0/1/2. Disabled kinds report zero.

The shared time step, integration, supports, history and preview APIs apply.
`zspace_solver_set_surface_params` supplies the shared positive step tolerance
(its area strength argument is unused by mode 3). Convergence requires all enabled
constraints within tolerance, max resultant below residualThreshold and full-frame
max step below stepTolerance. Force cancellation alone is not convergence.
All-disabled reports not reached. Fixed vertices participate in deviation checks.

Face forces use face-list storage directly, avoiding invalidated raw positions
and expensive halfedge reconstruction. Signed distances determine direction;
absolute distances determine tolerance. Preview snapshots clear particle forces
without clearing velocity and use actual magnitudes for component vectors.
Colours: face cyan, group violet, rigid pair red, resultant gold, supports black.
Nansha-specific constraint generation stays in the viewer sketch, not the kernel.

Test with `node wasm/scripts/test-planarity.mjs`. Optional reference and solve JSON
arguments also run the supplied Nansha dataset using the sibling viewer helper.

```text
bridge/   C++ bridge and compatibility headers exported to JavaScript.
scripts/  Local Emscripten setup and build wrappers.
out/      Generated `zspace_core.js` and `zspace_core.wasm` output.
```

`zspace_alice_webviewer` consumes the generated files from `wasm/out/`; it does
not own the C++ to WASM build.

The current bridge was promoted from the old Alice demo archive. It still
contains the transitional live-sketch entry points used by the viewer, but the
build ownership is now here so core API exports can be managed from the core
repo.

## Build

For the optional OpenUSD-enabled runtime, build a headless Emscripten SDK with
`wasm/scripts/build-openusd-sdk.ps1`, then run
`wasm/scripts/build-openusd-wasm.ps1`. The reusable SDK defaults to
`$env:USERPROFILE/source/sdks/OpenUSD/wasm-26.08`, outside the repository;
use `-OpenUSDRoot` to select another installed SDK. The SDK builder accepts
`-SdkRoot`, `-CacheRoot` and `-Rebuild`; source/build caches are separate from
the reusable installation.
This writes to `wasm/out/openusd/` and leaves the existing runtime intact.
See [OpenUSD IO](../docs/architecture/openusd-io.md) for dependencies, browser
threading requirements, and tests. The standard build below remains SDK-free.

From this repository root:

```bat
wasm\scripts\build-wasm.bat
```

The script requires CMake 3.21+ and Ninja on `PATH`, and expects Emscripten to
be installed at `%USERPROFILE%\emsdk` or available on `PATH` (for example,
after running `emsdk_env.bat`).

The standard SDK-free target is defined in `wasm/CMakeLists.txt`. The existing
batch entrypoint configures it with `emcmake`, then builds with Ninja. Project
objects and compiler-generated header dependencies are retained under
`wasm/build/standard/`. An unchanged invocation does no compilation or linking;
source edits rebuild their objects, header edits rebuild dependent objects,
and source additions/removals under `src/zCore`, `src/zInterface`, and `src/zIO`
automatically regenerate the build graph. The existing field/compute source
exclusions remain in effect. CMake/Ninja also track changed compile/link options.

The default uses the available logical processors, capped at sixteen jobs
(six if the processor count is unavailable). Set a positive integer to tune
memory use and throughput; use fewer jobs on machines with limited RAM:

```bat
set "ZSPACE_WASM_JOBS=4"
wasm\scripts\build-wasm.bat
```

Expensive translation units start early using object-library groups, while the
linker receives the original source order. A shared precompiled header (PCH)
reuses the bundled Eigen and JSON headers across those groups. Its generated
helper object is never linked into the runtime. Editing PCH inputs regenerates
the PCH and rebuilds its consumers; other headers retain their normal dependency
tracking. No-op configuration leaves the helper source and PCH unchanged.

PCH is enabled by default. To compare builds or troubleshoot a toolchain issue:

```bat
set "ZSPACE_WASM_USE_PCH=OFF"
wasm\scripts\build-wasm.bat
```

Set it to `ON` or clear the variable to restore the default. Changing this setting
automatically rebuilds affected objects. The original optimization, exception,
compiler definitions and linker options are preserved; PCH only adds the compiler
options needed to produce and consume the shared header artifact.

`ZSPACE_WASM_BUILD_DIR` and `ZSPACE_WASM_OUT_DIR` optionally select separate
build and output directories, useful for diagnostics. Use absolute paths and a
separate build directory for each runtime configuration. For example:

```bat
set "ZSPACE_WASM_BUILD_DIR=%CD%\wasm\tmp\diagnostic\build"
set "ZSPACE_WASM_OUT_DIR=%CD%\wasm\tmp\diagnostic\out"
wasm\scripts\build-wasm.bat
```

The wrapper deliberately uses no CMake build type or global C++ flags: the
target preserves `-O2`, C++17 and `-fexceptions`, without Release defaults such
as `-O3` or `-DNDEBUG`. Export discovery, runtime methods, browser environment
and memory/filesystem settings are unchanged. `EM_CACHE` holds Emscripten's
system libraries; Ninja's build directory holds project objects. After an
Emscripten upgrade, use a fresh build directory or clean the target first:

```bat
cmake --build wasm\build\standard --target clean
wasm\scripts\build-wasm.bat
```

The standard build and OpenUSD build use separate CMake projects and build
directories. No OpenUSD SDK or threading requirements are added to the
standard runtime. Generated files in `wasm/build`, `wasm/tmp`, `wasm/cache` and
`wasm/out` remain ignored by Git.

Generated files are written to:

```text
wasm/out/zspace_core.js
wasm/out/zspace_core.wasm
```

After building the default outputs, run the self-contained runtime regressions:

```bat
node wasm/scripts/test-planarity.mjs
node wasm/scripts/test-minimal-surface.mjs
node wasm/scripts/test-solver-display.mjs
node wasm/scripts/test-origami.mjs
```

## Minimal Surface Convergence

Mode 1 is a combined relaxation solver, not a pure mathematical minimal surface.
It adds the existing `zFnMeshDynamics::addMinimizeAreaForce` (SoapFilm area
gradient) to the same rest-length spring forces used by DR. Optional gravity
and vector loads participate too. Area forces use internal triangles; springs
use original input edges. The input mesh topology remains unchanged.

`zspace_solver_set_surface_params(areaStrength, stepTolerance)` sets independent
area strength and a positive full-frame displacement tolerance. Spring stiffness,
edge enable, rest-length multiplier, mass, and Euler/RK4 come from the existing
`zspace_solver_set_params` ABI. The guarded DR integrator and damping advance
the combined forces. Area-only descent/backtracking is deliberately not used:
combined relaxation may increase area to balance springs or loads.

Residual is the absolute magnitude of the summed area, spring and enabled load
forces, before integration limiting and excluding damping and support reactions.
Equilibrium requires BOTH maximum free-vertex resultant < residualThreshold AND
maximum free-vertex net displacement over the last frame < stepTolerance.
Defaults in the viewer are Area Strength 0.35, Spring Stiffness 1, Edge Force on,
Resultant Force Threshold 0.02 and Max Step Threshold 0.0001 (model length units).
Unlike the unchanged DR diagnostic, this residual is not a normalized force ratio.

Gold arrows show this same combined resultant; cyan area and red spring arrows
show the components. Display Forces enables components; Resultant Force additionally
enables their sum. Display Length Scale scales all arrows. Preview evaluation
restores particle forces and excludes support arrows. Curvature remains an
independent analysis, refreshed by the viewer; equilibrium no longer implies H=0
or a white curvature map. Legacy area-force tolerance exports remain for ABI
compatibility but no longer control mode 1 convergence; their residual is retired.

Run the runtime regression after building with `node wasm/scripts/test-minimal-surface.mjs`.

## Solver Display Convention

Colours use normalized RGB values and preserve the Dynamic Relaxation palette.
The resultant is always gold `(1, 0.72, 0)` in DR, minimal surface and origami.

| Force | RGB | Colour |
| --- | --- | --- |
| Resultant | 1, 0.72, 0 | Gold |
| Edge spring / origami axial | 1, 0.08, 0.02 | Red |
| Gravity | 0.05, 0.20, 1 | Blue |
| Vector load | 0, 0.72, 0.26 | Green |
| Area minimisation | 0, 0.75, 1 | Cyan |
| Origami crease | 1, 0, 0.65 | Magenta |
| Origami flat-facet hinge | 0.55, 0.20, 0.90 | Violet |
| Origami face-angle constraint | 0, 0.65, 0.65 | Teal |
| Origami damping | 0.45, 0.45, 0.45 | Grey |

Vectors are accumulated per free vertex per force type. Zero vectors are omitted.
Origami component sampling is optional via `zOrigamiForceComponents`; normal
stepping does not allocate these arrays. Its resultant includes damping, as before.
DR and minimal-surface resultant previews exclude damping, matching their residuals.
`Display Forces` hides all arrows without hiding supports or crease lines.

`zspace_solver_set_support_size(size)` accepts 4-32 screen-space pixels, default
24. It changes only support-marker point sizes, not masses, constraints or forces.
The viewer stores `supportSize` per object's solver, exposes it in Solver Parameters
(Origami Advanced), and supplies 24 for older scenes. Supports remain black.
Run `node wasm/scripts/test-solver-display.mjs` after rebuilding WASM.

## Solver Performance

The mesh dynamics bridge caches mesh edge endpoints during
`zspace_solver_create_dynamics()` and reuses that cache for spring forces,
residual diagnostics, and force previews. Removed solver edges stay in the cache
but are marked inactive, so Best Fit FDM and other mesh solvers can skip tension
edges without rebuilding mesh storage every frame.

Support and tension-edge edits can be driven incrementally from the viewer:
`zspace_solver_remove_support(vertexId)` removes a support marker and constraint,
`zspace_solver_remove_edge(edgeId)` marks an edge inactive for solver forces, and
`zspace_solver_restore_edge(edgeId)` re-enables it. `zspace_solver_remove_tension_edge`
and `zspace_solver_restore_tension_edge` are aliases for viewer code that labels
these inactive solver edges as tension edges. `zspace_solver_clear_removed_edges()`
restores every removed solver edge.

Origami `stepOrigami(settings, timeStep, diagnostics)` advances the complete
requested interval through stability-limited substeps, recomputing the bound
as the mesh moves. Previously it advanced only `min(timeStep, stableTimeStep)`
and discarded the remainder, slowing stiff or small-scale meshes in frame units.
The reported stable step is a numerical integration bound, not a displacement
limit. Origami Max Step is the largest net displacement over one full viewer
solver frame, not just its last substep. Neither quantity replaces the hinge,
strain, and speed convergence checks.

For the supplied nine-ring fixture, run
`node wasm/scripts/test-origami-circular-input.mjs path/to/CCF_06_circular.obj`.
This assigns 192 peak and 144 valley edges at 52% Fold and reports crease error,
panel bend, strain, speed, and the stable step. It tests progress and finite
geometry, not a claim that every soft crease target is simultaneously attainable.

The WASM build uses `-O2` for the bridge, sketch, and kernel sources. Do not ship
an `-O0` build for interactive dynamics; no fast-math flags are required.
Run `node wasm/scripts/benchmark-origami.mjs wasm/build/perf.json` from the
repository root to measure 432- and 1600-vertex annular meshes. The output includes
final positions for numerical comparison, as well as median and p95 frame times.
Compare timings on the same machine and compare positions before accepting an
optimization. This benchmark measures solver calls, not browser rendering FPS.

Viewer playback budgets complete updates (including geometry and analysis),
starting at one solver frame per redraw and capping adaptive batches at four.
Physics timesteps and convergence tolerances are independent of this scheduling.
