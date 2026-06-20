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
   In progress: mesh/graph iterator DLL-interface warnings are now suppressed
   at the private STL iterator members; remaining warning cleanup is focused on
   third-party QuickHull headers and `zParticle` float conversions.
9. Deprecate and later remove the `zObj*` compatibility names.

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

Topology-editing methods in `zFnMesh` are transitional compatibility APIs. New
geometry algorithms should operate on face-list data and replace topology in a
single validated operation. The remaining cleanup is to remove public
incremental half-edge editing and reduce internal maintenance to topology
construction, validation, cache invalidation, and legacy synchronization.

The following `zFnMesh` algorithms now operate directly on face-list storage:

- edge and face centers;
- face areas, mesh triangles, and volume calculations;
- triangle and quad matrix extraction;
- whole-mesh and single-face triangulation;
- isoline, iso-mesh, and isoband extraction with interpolated colors.

These operations support non-manifold input because they do not request the
lazy half-edge cache. Isoline and isoband extraction accepts triangles, quads,
and simple polygons rather than relying on quad-only marching-square cases.
