# zSpace Public API Migration

## Naming

New public types use keyword-first names:

- `zObject<Type>` owns object identity, transform, and private data.
- `zFn<Type>` creates, queries, and modifies object data.
- `zDisplay<Type>` stores appearance and visibility settings.
- `zDisplayScene` draws objects using reusable display settings and a render context.
- `zIt<Type><Component>` traverses object components.

The existing `zObj*` classes remain available during migration. Keyword-first
aliases and headers are the compatibility bridge until each object receives an
opaque implementation.

## Public Boundary

Public API consumers must not depend on `zMesh`, `zGraph`, or other storage
classes. Those types will move behind object implementations after function
sets and iterators no longer access public storage members.

Public headers will eventually contain only:

- object declarations and handles;
- function sets;
- iterators and non-owning views;
- lightweight math and value types;
- result and error types;
- display settings and renderer interfaces.

Geometry storage, topology containers, file-format implementations, OpenGL,
libigl, USD, and host SDK dependencies belong in private source directories.

## Migration Order

1. Make polymorphic bases safe and normalize inheritance.
2. Introduce keyword-first names without breaking existing callers.
3. Add validated function-set attachment APIs.
4. Move mesh and graph drawing into display classes and `zDisplayScene`. Completed.
5. Place mesh and graph storage behind opaque object implementations. Completed.
6. Update iterators to attach through object handles or private accessors. Completed for mesh, graph, and point-cloud public iterators.
7. Repeat the pattern for point clouds, particles, fields, and spatial bins. In progress:
   public function-set signatures now use `zObject*` names for mesh, graph,
   point-cloud, point-field, mesh-field, and dynamics APIs. Field and particle
   storage members are now private and reachable only by the owning function
   sets and iterators.
8. Remove storage headers and third-party dependencies from the installed API.
   In progress: mesh/graph/field iterator and field-function-set
   DLL-interface warnings are suppressed at private STL storage members.
   Bundled third-party include paths are treated as system includes by CMake,
   and the `zParticleDerivative` scalar multiplication warning/assignment bug
   has been fixed.
9. Deprecate and later remove the `zObj*` compatibility names.

## Active Checklist

Update this section after each migration step.

- Mesh face-list API cleanup: completed. Public incremental half-edge
  construction methods have been removed from `zFnMesh`; private temporary
  helpers remain only for subdivision/smoothing until those algorithms are
  face-list-native.
- Mesh face-list algorithm audit: in progress. Vertex averaging, vertex/face
  color smoothing, and mesh dynamics boundary fixing now operate on face-list
  edges and face boundary counts instead of asking the lazy half-edge cache.
  Mesh extrusion and boundary-edge extrusion now derive boundary sides from
  face-list edge incidence. Convex-hull half-space checks, triangle/quad mesh
  classification, edge and half-edge-length compatibility queries, edge
  dihedral angles, principal-curvature adjacency setup, planarity deviation,
  and boundary-excluded edge data now read face-list storage directly.
  Dual graph/mesh extraction, rainflow graphs, edge-loop tools,
  subdivision/smoothing, and mesh dynamics topology force helpers remain
  topology-dependent.
- Mesh source cleanup: completed. Inactive `#if 0` blocks for removed
  delete/collapse/flip/split-face topology editing have been deleted.
- Field API simplification: in progress. Public scalar/vector field function
  set classes now exist for mesh and point fields. Scalar-only methods are
  exposed through `zFnMeshScalarField` / `zFnPointScalarField`; vector-field
  creation from scalar fields is exposed through `zFnMeshVectorField` /
  `zFnPointVectorField`. The shared templates have been renamed to
  `zFnMeshFieldBase<T>` / `zFnPointFieldBase<T>` as implementation bases.
  `zFnMeshField<T>` and `zFnPointField<T>` remain deprecated compatibility
  aliases during migration. Mesh scalar/vector wrappers now expose normal field
  value queries directly, so new code does not need to reference the base
  templates for common field reads. Mesh and point scalar/vector wrappers now
  explicitly re-export common create/query/set methods, and
  `zspace/interface.h` includes field objects, iterators, and function sets.
  Smoke coverage now verifies mesh scalar fields, mesh vector fields generated
  from scalar fields, point scalar fields, and compile-level point vector field
  access. Point vector field runtime creation needs a dedicated follow-up pass
  before it is added to the smoke runtime assertions.
  Mesh scalar-field iso contour/isoline/isoband methods are now lightweight
  wrappers over `zFnMesh`; the old field-local marching-squares helper
  implementation has been removed.
- Graph edge-list storage: completed first pass. `zObjectGraph` now owns
  edge-list storage as the authoritative representation and lazily builds the
  legacy half-edge `zGraph` cache for iterator/topology operations. Common
  `zFnGraph` create/count/attribute/bounds/edge-data/center/length/transform
  methods use the edge-list path. Vertex color averaging from edge colors,
  graph vertex averaging, graph eccentricity center queries, half-edge-shaped
  center/length compatibility queries, graph IO export, and graph dynamics
  boundary fixing are now edge-list-native.
  `zItGraphVertex` and `zItGraphEdge` now use edge-list
  indices for simple iteration, position/color access, endpoint queries, and
  edge length/center/vector queries; `zItGraphHalfEdge` remains the explicit
  topology path.
- Graph topology editing cleanup: in progress. Legacy graph `addEdges`,
  `splitEdge`, and inactive-element removal are no longer public API methods;
  they remain private topology helpers while legacy topology-dependent code is
  phased out. Graph dynamics angle forces now use edge-list adjacency for
  hinge lookup.
- Public half-edge count cleanup: completed. `zFnMesh::numHalfEdges()` and
  `zFnGraph::numHalfEdges()` are no longer public function-set methods.
  Half-edge counts are topology details and should be queried through
  `zItMeshHalfEdge::size()` / `zItGraphHalfEdge::size()` only when code is
  explicitly using half-edge traversal.
- Public edge query cleanup: completed. `zFnMesh` and `zFnGraph` now expose
  edge-list-facing `edgeExists(v1, v2, edgeId)`. `halfEdgeExists(...)` is no
  longer public and remains only as a protected topology helper for internal
  and interop code that explicitly works with half-edge traversal.
- Static geometry API cleanup: completed. Public `zFnMesh::makeStatic()`,
  `zFnGraph::makeStatic()`, static create flags, and the `zIO::readMesh` /
  `zIO::readGraph` static-geometry flags have been removed. Meshes and graphs
  are stored in their compact primary representations; legacy static
  half-edge caches are not part of the public API.
- Raw pointer/public dependency audit: pending. Keep performance escape hatches
  only where needed and document them as advanced API.
- Public header warning cleanup: in progress. Field STL storage and iterator
  members are isolated from the DLL export surface with targeted warning
  suppression; bundled dependency headers are now marked `SYSTEM` in CMake.
- Smoke test expansion: in progress. `zspace_smoke` now covers non-manifold
  face-list creation, face-list mesh algorithms, extrusion, topology-based dual
  graph extraction, scalar/vector mesh fields, point scalar fields, point
  vector field compile access, point clouds, and transformation copy behavior.
- Interop naming cleanup: completed first pass. Active InterOp function sets,
  converters, and Display internals now use `zObject*` names; `zObj*`
  remains only as compatibility aliases and migration documentation.

## Current Transition Rules

- New application code should include `zObject*.h` and use `zObject*` names.
  The `zObj*` headers remain as compatibility aliases only.
- New code should mutate geometry through `zFn*`.
- New code should not access `.mesh`, `.graph`, or related storage directly.
- Rendering code should not be added to object classes.
- Object-level drawing methods and display setters have been removed from the migrated objects.
- Public mesh and graph object headers no longer include or expose `zMesh` and
  `zGraph`. SDK installation rules will be added after remaining public-header
  dependencies are separated.
- DLL import/export macros now keep core storage symbols out of the interface
  DLL export surface.

## Rendering

Mesh and graph appearance is configured independently from geometry:

```cpp
zObjectMesh mesh;
zFnMesh meshFn(mesh);

zDisplayMesh display;
display.setElements(false, true, true);
display.setVertexNormalsVisible(true, 0.25);

zDisplayScene scene(displayUtils);
scene.draw(mesh, display);
```

New code should use reusable display styles with `zDisplayScene`.

## Mesh Storage

`zObjectMesh` now uses compact face-list arrays as its authoritative storage:

- vertex positions;
- face offsets and face-vertex indices;
- derived unique edge endpoint pairs;
- vertex, edge, and face attributes.

The previous half-edge mesh is an internal lazy topology cache. Geometry IO,
display, bulk function-set methods, and vertex/edge/face iteration do not build
that cache. A half-edge-dependent query builds and reuses it automatically for
a consistently oriented manifold mesh. Such a query reports an error for a
non-manifold mesh instead of rejecting the face-list geometry itself.

`zItMeshVertex`, `zItMeshEdge`, and `zItMeshFace` are index-backed facades over
the face-list representation. Their local geometry and attribute access works
for manifold and non-manifold meshes. Half-edge traversal remains available
through topology-dependent methods and `zItMeshHalfEdge` during migration.

Incremental topology-editing methods have been removed from the public
`zFnMesh` API. New geometry algorithms should operate on face-list data and
replace topology in a single validated operation. Internal half-edge
maintenance is limited to topology construction, validation, cache
invalidation, legacy synchronization, and temporary subdivision/smoothing
helpers during migration.

The following `zFnMesh` algorithms now operate directly on face-list storage:

- edge and face centers;
- face areas, mesh triangles, and volume calculations;
- triangle and quad matrix extraction;
- whole-mesh and single-face triangulation;
- isoline, iso-mesh, and isoband extraction with interpolated colors;
- vertex averaging and vertex/face color smoothing;
- mesh dynamics boundary fixing;
- extrusion and boundary extrusion side-wall generation.
- convex-hull half-space checks;
- triangle/quad mesh classification;
- edge and half-edge-length compatibility queries;
- edge dihedral-angle queries;
- principal-curvature adjacency setup;
- per-face planarity deviation;
- boundary-excluded edge-data queries.

These operations support non-manifold input because they do not request the
lazy half-edge cache. Isoline and isoband extraction accepts triangles, quads,
and simple polygons rather than relying on quad-only marching-square cases.

## Graph Storage

`zObjectGraph` now uses compact edge-list arrays as its authoritative storage:

- vertex positions;
- edge endpoint pairs;
- vertex and edge colors;
- vertex and edge weights.

The previous half-edge graph is an internal lazy topology cache. Graph IO,
bulk function-set methods, display, edge length/center queries, simple
vertex/edge iteration, endpoint queries, color interpolation, graph smoothing,
eccentricity centers, graph IO export, graph dynamics boundary and angle-force
adjacency, and transform updates should operate on edge-list data. Half-edge
iterators, ordered connected-edge traversal, graph mesh widening, and other
topology-dependent operations build and reuse the cache automatically.

Dual mesh and dual graph extraction are intentionally topology-based.

New graph algorithms should prefer the edge-list representation unless they
need ordered half-edge traversal around vertices. If an algorithm mutates the
lazy topology cache, the storage bridge syncs the edge-list representation
before later edge-list reads.
