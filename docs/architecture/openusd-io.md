# OpenUSD mesh IO

OpenUSD replaces TinyUSDZ behind the existing `zIO::readMesh` and
`zIO::writeMesh` signatures. SDK headers and types stay private to IO; Core,
Interface, and Display do not link OpenUSD. The legacy Omniverse InterOp switch
`ZSPACE_WITH_USD` is independent of this codec.

## Native build

The SDK-free default uses `ZSPACE_IO_WITH_OPENUSD=OFF`. USD extensions return
an explicit disabled-support error; other codecs remain available.

For a standard SDK installation with `pxrConfig.cmake`:

```powershell
cmake --preset ninja-msvc -DZSPACE_IO_WITH_OPENUSD=ON -DZSPACE_OPENUSD_ROOT="C:/SDKs/OpenUSD"
cmake --build --preset ninja-msvc-release --parallel
```

Use a headless SDK built without Python and imaging where possible. Keep the
SDK's DLL/shared-library paths and plugin/schema resources available at runtime.
Do not copy just the import libraries. SDK distribution and licenses remain the
application packager's responsibility; the codec does not bundle an SDK.

Windows vendor devkits that separate headers/import libraries from the runtime
can instead configure `ZSPACE_OPENUSD_RUNTIME_ROOT` alongside
`ZSPACE_OPENUSD_ROOT`. This chooses direct linking of the IO libraries rather
than the vendor's full Python/imaging CMake package. For Python-enabled devkits,
also supply `ZSPACE_OPENUSD_EXTRA_INCLUDE_DIR` (Python headers) and
`ZSPACE_OPENUSD_EXTRA_LIBRARY_DIR` (Python import-library directory). Matching
Python runtime DLLs must be on PATH. This does not link Maya APIs.

The old `ZSPACE_IO_WITH_TINYUSDZ` option no longer selects a backend. Update
existing configure commands to use the OpenUSD options above.

## Compatibility

- Read the first composed mesh in traversal order, in object space, at default
  time. Do not bake transforms or convert units/up-axis in `readMesh`.
- Export `/Mesh`, polygon geometry (`subdivisionScheme = "none"`), Z-up and
  centimetres, retaining the previous coordinate contract.
- `.usda` and `.usd` exports are text. `.usdc` is crate; `.usdz` packages a
  temporary crate layer that is removed after packaging.
- Preserve vertex colors, uniform face normals, face colors, edge endpoint
  pairs, edge colors/weights, and `zspace.mesh.v1` layer metadata.
- Flatten indexed primvars. Constant display colors expand to vertices;
  uniform display colors map to faces. Face-varying colors/normals and UVs are
  not represented by the current mesh IO data contract.
- Write canonical `zspace:faceOpacity` and `zspace:edgeOpacity`; also read old
  TinyUSDZ `faceColorOpacity`/`edgeColorOpacity` spellings.
- Validate USD topology before creating the destination mesh. Parser/topology
  errors return `zIOResult` and leave the destination unchanged.

The additive internal scene codec and WASM document exports are independent of
the single-mesh API. `readSceneUSD` traverses composed polygon meshes (including
native instance proxies), bakes world transforms, corrects handedness and converts
stage units/up-axis to centimetres/Z-up. `writeSceneUSD` writes separate named,
visible polygon meshes in one stage. Internal `USDSceneData` / `USDMeshData`
hold geometry in typed zSpace arrays without serialization. The old document
overloads remain for compatibility; the viewer does not use them. Unresolved
composition dependencies fail before publishing a scene. Animation, point instancers, materials, UVs and
camera/light translation are not implemented; hierarchy is flattened.

## WASM

The SDK must be compiled for Emscripten; Windows DLLs/import libraries cannot
be reused. The development SDK build is pinned to OpenUSD v26.08
(`ee47c679abde5b467a7b6a41f3b2285564a4222e`) and oneTBB 2021.12.0.
See `wasm/scripts/build-openusd-sdk.ps1` for a reproducible headless SDK build.

```powershell
.\wasm\scripts\build-openusd-sdk.ps1
.\wasm\scripts\build-openusd-wasm.ps1
node wasm/scripts/test-openusd.mjs
node wasm/scripts/test-openusd-buffers.mjs
```

Core resolves the runtime SDK from its built-in layout or local configuration;
see [SDK configuration](../development/openusd-sdk.md). The runtime builder
requires no SDK path. The separate SDK builder accepts `-SdkRoot` and
reuses an installed SDK unless `-Rebuild` is supplied. New source downloads,
dependency sources and compiler intermediates go under
`$env:USERPROFILE/source/sdks/OpenUSD/cache/wasm-26.08` (override with `-CacheRoot`).
That cache can be removed after a successful SDK installation; keep the SDK's
headers, archives, CMake files and embedded schema/plugin resources.

On this workstation, the native MayaUSD 25.5 devkit is installed at
`C:/Users/vishu.b/source/sdks/OpenUSD/native-maya-25.5`. Its runtime DLLs still
come from the MayaUSD installation named by `ZSPACE_OPENUSD_RUNTIME_ROOT`.
Neither SDK is committed or copied into the viewer. Only the generated runtime
pair in `wasm/out/openusd` is needed for viewer packaging.

The OpenUSD-enabled core runtime is written to `wasm/out/openusd/`, leaving the
ordinary `wasm/out/` output untouched. It uses the same codec and existing
`zspace_mesh_read`/`zspace_mesh_write` bridge. OpenUSD's CMake targets embed USD
schema/plugin resources. Threading requires SharedArrayBuffer and browser
COOP/COEP headers on the page and module-worker responses. The v26.08 SDK's
workers load the same generated ES module, so the JS/WASM pair must be packaged
together. Import the threaded glue from its served URL rather than a temporary
blob URL, which cannot resolve its relative module-worker URL.

The sibling viewer uses `/wasm/openusd/zspace_core` as a separate IO runtime,
preserving its ordinary kernel and solver state. Its
`scripts/sync-openusd-runtime.ps1` copies these outputs to ignored
`wasm/openusd/`; its USD File-menu and clipboard actions use typed scene
buffers. No JSON encoding/decoding or intermediate mesh files occur on this
path. Owned typed sources are retained in the viewer for history replay and
source-path replacement. USD import leaves the primary active mesh/solver
untouched. The build itself does not copy outputs into other repositories.

At app startup, USD loading starts alongside ordinary app loading. Its readiness
promise includes a tiny stage read to initialize schemas, parsers and first-use
SDK work, and is awaited before the render loop. Concurrent imports await that
same cached promise. USD startup failure lets ordinary app loading continue;
subsequent USD requests can retry.

The browser host explicitly applies `WorkSetConcurrencyLimit(3)` after SDK/TBB
static initialization, before the first stage read. This limits worker spawning
in the isolated USD runtime, which has a small preallocated pthread pool. It
does not change native IO defaults or the primary viewer runtime. A pre-static
initialization environment override failed validation and is not shipped.
The budget favors startup overhead; benchmark large-scene throughput before
increasing parallelism.

### Typed scene ABI

- `zspace_usd_initialize()` warms the SDK without changing active geometry.
- `zspace_usd_scene_open(path)` retains a validated composed scene.
- `zspace_usd_scene_mesh_count()` and scene name/path/visibility/warning getters
  expose metadata and UTF-8 strings.
- `zspace_usd_scene_select(index)` builds one mesh into independent buffers.
  `zspace_usd_buffer_ptr(slot)` / `_count(slot)` expose pointers/scalar counts.
  Float32 slots 0-7: positions, normals, vertex RGB, triangle RGB, face centers,
  face normals, edge centers, edge weights. Uint32 slots 8-11: triangles, edge
  endpoints, polygon counts, polygon connects.
- `zspace_usd_scene_add_mesh(name,path,visible,positions,positionCount,counts,
  faceCount,connects,connectCount)` copies typed geometry. Attribute upload via
  `zspace_usd_scene_set_attribute(index,slot,pointer,count)` accepts vertex RGBA
  (0), polygon RGBA (1), Uint32 edge pairs (2), edge RGBA (3), Float32 weights (4).
  Counts, indices and finite values are validated.
- `zspace_usd_scene_save(path)` writes the requested USD format.
  `zspace_usd_scene_clear()` releases the scene and clears readback buffers.

Copy readback to owned JS arrays before selecting/clearing another mesh. Refresh
heap views after allocations because memory can grow. Upload allocations can
be freed after their call returns. Legacy document exports remain available for
compatibility, but the viewer has no JSON fallback.

Use an existing virtual directory such as `/tmp` or a viewer-created `/output`
for exports. OpenUSD v26.08's atomic writer rejects files directly at the
virtual filesystem root (for example `/mesh.usda`).

Validation on Windows used the MayaUSD OpenUSD 25.5 devkit/runtime for native
IO and a headless OpenUSD 26.08 SDK for WASM. Native Release smoke tests passed
with USD enabled and disabled. The WASM test exercises all four formats in
Node through the shared C++ codec, including Maya-style scene units/transforms,
mirrored winding, visibility and unresolved references. Browser tests separately
verified threaded loading and viewer clipboard round trips with Maya-style
synthetic USDA. Direct-buffer tests also check startup/solver isolation,
owned readback lifetimes, colors and all formats without JSON files. A running
Maya plugin was not exercised.

## Import performance audit

The 2026-10-08 Nansha roof audit used the viewer's actual runtime pair and
the supplied Desktop OBJ (293,181 bytes) / binary USD (23,071 bytes). Both
produced 1,600 vertices, 1,440 polygons and 2,880 render triangles.
An isolated browser page measured the original first USD import at 883 ms
versus 121 ms for OBJ; subsequent USD imports were 28-37 ms versus 80-89 ms
for OBJ. Timings exclude scene-object creation and GPU rendering.

The main difference is lazy OpenUSD initialization: an 11.3 MB runtime,
pthread workers, schema/plugin registration and first-use compilation. The
ordinary 0.67 MB runtime is already available for OBJ. Node runs with a new
stage identifier on every iteration confirmed that the warm improvement is
not just reuse of the roof layer. Pre-opening an unrelated three-vertex USD
absorbed approximately 438 ms of first-stage work, leaving the roof's first
scene decode/JSON export at 19 ms. Warm Node scene decode/JSON export was
approximately 4-6 ms; the mesh JSON reinstall and source retention accounted
for most of the remaining 22-36 ms USD import path.

Browser resource timings also found an avoidable duplicate 11.3 MB WASM
download: this SDK's generated glue ignores `wasmBinary`. The viewer's
threaded loader now imports the served glue directly and lets its built-in
asynchronous streaming loader fetch one revision-matched binary. The final
saved browser run verified one WASM fetch, 615 ms first USD import and
25-27 ms warm imports, versus OBJ's 81 ms first and 71-88 ms warm imports.
These are individual local measurements, not a controlled percentage gain:
first-use timings vary with compiler caches and concurrent system load.
Initialization remains a one-time cost per page/runtime; idle prewarming
could move it earlier. Removing the JSON round trip is a separate potential
optimization for large scenes, not the cause of this roof's cold-load delay.

The subsequent implementation moves readiness into app loading and removes
that JSON round trip. The saved bounded-worker browser run measured startup at
887 ms, followed by roof USD imports at 23 ms / 7-8 ms, versus OBJ at 82-86 ms.
These exclude scene creation/GPU rendering and are local samples, not a
controlled percentage gain. The browser audit successfully imports/exports
all formats with JSON parsing/serialization disabled during USD exchange and
checks independent typed source retention. Worker module requests dropped to
three worker bootstraps plus one main-module load for the complete audit.


Reproduce the Node audit from the core repository (viewer dependencies must
be installed):

```powershell
node wasm/scripts/benchmark-usd-import.mjs ../zspace_alice_webviewer C:/Users/vishu.b/Desktop/Geometries/nansha_roof.obj C:/Users/vishu.b/Desktop/Geometries/nansha_roof.usd build/usd-import-timings.json
# Append --prewarm after the output filename for the unrelated-tiny-stage control.
```

For actual browser loading, start the viewer at port 5192, then run
`benchmark-usd-browser.mjs` with the OBJ and USD paths. It serves a temporary,
localhost-only diagnostic page at port 5193 and proxies the running viewer's
modules; it does not alter the user's viewer scene. Press Run import comparison.
The process should be stopped after the audit. Module-worker JS resource
durations can be reported with incompatible clock origins; compare main binary
fetch counts and the enclosing import durations rather than those worker entries.

## Tests

### Generic document API

`USDdoc` (`<zspace/zIO/zUSDapi.h>`, linked through `zspace::io`)
provides default-time USD document queries and authoring. Unlike the single-mesh
codec, it enumerates prims and attributes, returns world-transformed points and
flattens indexed primvars. It contains no slicer names, settings or unit policy.
Native C++, the `USDdoc_*` WASM bridge and Python's
`zspace.core.USDdoc` use this same C++ implementation.

- `open`, `close`: own a detached root-layer copy, leaving the source unchanged.
- `readMetadata`: `zUSDMetadata` with units, up axis and prim records.
- `readPrim`: `zUSDPrimInfo` with path, type, attribute names and reflection flag.
- `readAttribute`: `zUSDAttribute` with exact USD type, typed value, custom flag
  and interpolation. Indexed primvars are expanded; XYZ arrays are flat.
- `readCustomLayerData`: a map of keys to typed values.
- `readPoints`: world-space XYZ doubles in source units.
- `applyEdits`: a `zUSDEdits` transaction with removals, prim/attribute records
  and custom-layer-data changes. Failed edits leave the document unchanged.
- `save` and `text`: explicitly export the root layer or return USDA text.

The old JSON methods are removed, with no compatibility aliases. `zUSDData`
uses a C++ variant: bool, int32, uint32, float32, float64, string and typed
int/uint/float/double/string arrays. Tokens retain their USD type but carry strings;
point/normal/vector triples retain their role and float/double precision.
Nested dictionaries, matrices and other unsupported types fail explicitly.
Type/payload mismatches, nonfinite authoring values and invalid vector lengths
fail before commit. Queries use default time; this is not an animation API.

Python passes dictionaries and lists directly through pybind, without JSON
serialization. Numeric sequences are copied to typed C++ vectors; this is not
zero-copy NumPy IO. WASM transfers numeric arrays in typed linear-memory buffers
and exchanges strings separately. JavaScript objects are not serialized.
The browser wrapper copies returned buffers before the next bridge call and
frees upload buffers after the bridge copies them. Separate WASM instances still
have separate memories. No unit or up-axis conversion occurs inside this API.
The bridge owns one active document; callbacks must remain synchronous.
Rebuild Core, Python bindings and the matching WASM runtime before using these
new methods; old binaries do not provide the new ABI.

Saving does not package external assets or rewrite their relative paths; keep
the referenced files reachable when relocating an exported layer. Browser IO
uses the WASM filesystem, so referenced files must also be present there. Use
`/tmp/output.usda` for browser exports, then read its bytes for download.

Slicer interpretation belongs to the host adapters: Alice's `userSrc/zspace/slicer/SlicerIO.cpp`,
the viewer's `slicingUSD.ts`, and Python's `zspace.io.slicing_usd`. These convert
geometry and dimensional settings to Z-up millimetres, preserve ratios and
export results back in source units. Toolsets links only Core/Interface and
does not include or link OpenUSD. The desktop adapter retains 500-character
USDA array wrapping; whitespace is not part of the shared schema.

`zspace.io.smoke` is backend-aware: OFF tests disabled-support reporting, ON
tests all four extensions, opacity, standard normals, legacy attribute names,
indexed colors, reference composition, object-space first-mesh behavior and
invalid topology preserving the destination. Run the full Release build and
CTest with the SDK's runtime dependencies available.

`readPoints` returns `std::vector<std::array<double, 3>>` in C++ and a list
of XYZ triples in Python. The WASM bridge flattens these triples for its numeric
buffer contract, so the browser API still returns a flat coordinate array.


### Typed attribute access

`USDdoc` exposes `readBool`, `readInt`, `readFloat`, `readDouble`,
`readString`, `readIntArray`, `readFloatArray`, and `readDoubleArray`.
The floating-point readers accept USD float or double values of the same
scalar/array shape, with finite-value and overflow checks. Integer and boolean
readers require their corresponding USD types. `readString` accepts string and
token attributes. Missing required values and incompatible types throw.

Each reader has a `tryRead?` counterpart. In C++, a missing attribute returns
`false` without changing the output argument; Python returns `None`, and the
TypeScript wrapper returns `undefined`. An existing empty array is a valid value;
applications may choose to retain their defaults for it. A missing prim remains
an error.

The matching static `write?` methods add typed attributes to a prim edit, without
performing IO. Call `applyEdits` once to validate and commit the batch atomically.
Python uses a prim dictionary and TypeScript a prim object; neither serializes
JSON. The C++ representation is `zUSDPrimEdit`.

```cpp
float spacing = 8.0f;
doc.tryReadFloat("/Slicer_setting", "slicer:LayerSpacing", spacing);
zUSDEdits edits;
edits.prims.push_back({"/Result", "Scope", {}});
USDdoc::writeFloat(edits.prims.back(), "spacing", spacing);
doc.applyEdits(edits);
```


## Object access API

`USDdoc.getPrimAtPath(path)` returns a lightweight `USDPrim` handle.
`prim.getAttribute(name)` returns a `USDAttribute` handle. `isValid()` tests
existence, not whether an attribute has an authored value. Handles expire when
 their document is destroyed, closed or reopened. A missing value throws on `get()`.

```cpp
USDdoc doc;
doc.open("input.usda");
auto settings = doc.getPrimAtPath("/Slicer_setting");
double spacing = settings.getAttribute("slicer:LayerSpacing").get<double>();
auto output = doc.definePrim("/TDP_gcode", "Scope");
output.createAttribute("printWidths", "float[]").set(widths);
doc.save("output.usda");
```

Python and JavaScript expose the same lower-camel-case method names, with
`attribute.get()` returning the stored value and `attribute.set(value)` selecting
conversion from the attribute's USD type. C++ additionally provides `get<T>()`.
Float/double reads allow checked conversion; other mismatches throw. Vector
attributes retain their USD type/role but use flat XYZ numeric arrays at this
boundary. The explicit `readWorldPoints(path)` convenience returns transformed
points (C++/Python triples; JavaScript flat triples) in source units and up axis.

`get()` reads default-time, local, raw attribute values. It does **not** flatten
indexed primvars or apply transforms, unit scaling or up-axis changes. Existing
`readAttribute()` retains its legacy primvar-flattening behavior. This API is a
subset of PXR, not a complete replacement for schemas, layers or time samples.

`createAttribute()` authors a declaration, and `set()` modifies the open stage
immediately without saving to disk or cloning the full document. `save()` exports
its root layer. Sequential writes are not a transaction: a later failure leaves
successful earlier writes in memory. The existing `applyEdits()` batch API remains
available for callers requiring rollback on failure. `setCustomLayerData()`
replaces the dictionary; read and merge it first to preserve unrelated metadata.

Python bindings are conditional on `ZSPACE_PYTHON_WITH_USD`; Blender's existing
USD-disabled build continues using its native `pxr` module. The WASM C bridge
adds direct operations alongside the existing batch protocol; its document is
still single-instance and browser callbacks must remain synchronous.
