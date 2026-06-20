# Querying And Using The zSpace API

This guide is for humans and Codex/LLM agents answering questions about
existing zSpace methods, finding usage patterns, and writing small example
snippets.

## Public API Mental Model

Use this pattern first:

```cpp
zObjectMesh mesh;
zFnMesh fn(mesh);
```

Objects own data and identity. Function sets create, edit, and query geometry.
Display classes own rendering state. IO classes read and write external formats.

Keyword-first names are the current public style:

```text
zObject<Type>       Geometry/data object with identity and private storage.
zFn<Type>           Function set for creating, editing, and querying an object.
zIt<Type><Part>     Iterator over components of an object.
zDisplay<Type>      Display settings and draw behavior for an object type.
zCodec<Format>      Format-specific IO codec.
zIO                 Stateless facade for common read/write operations.
```

Compatibility names such as `zObjMesh` may exist, but new snippets should use
`zObjectMesh`.

## Where To Look

Useful source locations:

```text
include/zspace/zInterface/functionsets/    Public method declarations.
src/zInterface/functionsets/               Method implementations.
include/zspace/zInterface/objects/         Public object declarations.
src/zInterface/objects/internal/           Private storage/impl details.
include/zspace/zIO/                        IO facade and codec declarations.
src/zIO/                                   IO facade and codec implementations.
include/zspace/zDisplay/                   Display API declarations.
src/zDisplay/                              Display implementation.
tests/smoke/                               Minimal usage examples.
docs/architecture/api-migration.md         Current migration state.
agents/extending_zspace_core.md            Rules for adding new code.
```

Prefer `rg` before opening large files:

```powershell
rg "class ZSPACE_API zFnMesh" include src
rg "getPrincipalCurvatures" include src tests
rg "zIO::readMesh|readMesh\\(" include src tests
rg "zDisplayMesh" include src tests docs agents
rg "zFnMeshScalarField|zFnPointScalarField" include src tests docs agents
rg "zFnGraph|zObjectGraph" include src tests docs agents
```

## Answering API Questions

When answering a usage question:

1. Search declarations in `include/zspace/...`.
2. Search implementations in `src/...`.
3. Check `tests/smoke/` for a compiling example.
4. Prefer public umbrella headers in snippets:
   - `#include <zspace/interface.h>`
   - `#include <zspace/io.h>`
   - `#include <zspace/display.h>`
5. Use `zObject*` names in examples.
6. Keep snippets small and focused.
7. Mention whether the method belongs to object, function set, IO, or display.

## Build And Test Commands

Build from PowerShell:

```powershell
.\scripts\build.ps1 -Configuration Release
```

Build from Command Prompt:

```bat
scripts\build_core_interface.bat
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

## Mesh Examples

Create and query a mesh:

```cpp
#include <zspace/interface.h>

zSpace::zObjectMesh mesh;
zSpace::zFnMesh fn(mesh);

zSpace::zPointArray positions;
zSpace::zIntArray faceCounts;
zSpace::zIntArray faceConnects;

fn.create(positions, faceCounts, faceConnects);

zSpace::zPoint minBB;
zSpace::zPoint maxBB;
fn.getBounds(minBB, maxBB);
```

Read a mesh and query bounds:

```cpp
#include <zspace/interface.h>
#include <zspace/io.h>

zSpace::zObjectMesh mesh;
auto result = zSpace::zIO::readMesh("input.obj", mesh);

if (result)
{
    zSpace::zFnMesh fn(mesh);
    zSpace::zPoint minBB;
    zSpace::zPoint maxBB;
    fn.getBounds(minBB, maxBB);
}
```

Write a mesh:

```cpp
#include <zspace/interface.h>
#include <zspace/io.h>

zSpace::zObjectMesh mesh;
zSpace::zIO::writeMesh("output.json", mesh);
```

## IO Examples

Use `zIO` for user-facing file operations:

```cpp
#include <zspace/interface.h>
#include <zspace/io.h>

zSpace::zObjectMesh mesh;

auto readResult = zSpace::zIO::readMesh("input.obj", mesh);
if (!readResult)
{
    std::cerr << readResult.message() << std::endl;
}

auto writeResult = zSpace::zIO::writeMesh("output.json", mesh);
if (!writeResult)
{
    std::cerr << writeResult.message() << std::endl;
}
```

Graph IO follows the same facade style:

```cpp
#include <zspace/interface.h>
#include <zspace/io.h>

zSpace::zObjectGraph graph;

zSpace::zIO::readGraph("graph.txt", graph);
zSpace::zIO::writeGraph("graph.json", graph);
```

Do not suggest `fromJSON`, `toJSON`, `fromOBJ`, or `toOBJ` methods on function
sets for new code. Format-specific behavior belongs in `zCodec<Format>`.

## Graph Examples

Create and query an edge-list graph:

```cpp
#include <zspace/interface.h>

zSpace::zObjectGraph graph;
zSpace::zFnGraph fn(graph);

zSpace::zPointArray positions = {
    zSpace::zPoint(0, 0, 0),
    zSpace::zPoint(1, 0, 0),
    zSpace::zPoint(1, 1, 0)
};
zSpace::zIntArray edgeConnects = { 0, 1, 1, 2 };

fn.create(positions, edgeConnects);

zSpace::zDoubleArray edgeLengths;
fn.getEdgeLengths(edgeLengths);
```

Iterate graph edges:

```cpp
#include <zspace/interface.h>

zSpace::zObjectGraph graph;

for (zSpace::zItGraphEdge edge(graph); !edge.end(); edge++)
{
    zSpace::zIntArray vertices;
    edge.getVertices(vertices);
}
```

Use `zFnGraph` for graph creation, counts, bounds, positions, colors, weights,
edge connectivity, centers, lengths, and transforms. `zObjectGraph` stores an
edge list as its primary representation; half-edge graph topology is built
lazily when code asks for ordered traversal through graph iterators or
topology-specific methods.

Use `zItGraphVertex` and `zItGraphEdge` for simple indexed traversal and local
geometry/attribute access. Use `zItGraphHalfEdge` only when the question is
specifically about ordered graph topology.

Avoid examples that mention direct `zGraph` storage.

## Display Examples

Display a mesh:

```cpp
#include <zspace/interface.h>
#include <zspace/display.h>

zSpace::zObjectMesh mesh;
zSpace::zDisplayScene scene;

zSpace::zDisplayMesh& display = scene.mesh(mesh);
display.setDisplayElements(false, true, true);
display.setDisplayVertexNormals(true, 0.5);

scene.draw(mesh);
```

Display state belongs to `zDisplay*` classes, not to `zObject*` classes.
Avoid examples that call draw methods on geometry objects.

## Point Cloud Examples

Create a point cloud:

```cpp
#include <zspace/interface.h>

zSpace::zObjectPointCloud points;
zSpace::zFnPointCloud fn(points);

zSpace::zPointArray positions;
fn.create(positions);
```

Iterate point cloud vertices:

```cpp
#include <zspace/interface.h>

zSpace::zObjectPointCloud points;

for (zSpace::zItPointCloudVertex v(points); !v.end(); v++)
{
    zSpace::zPoint* p = v.getRawPosition();
}
```

## Field Examples

Use scalar/vector-specific field function sets in new snippets. The template
function-set names are compatibility aliases only.

Create and query a 2D mesh scalar field:

```cpp
#include <zspace/interface.h>

zSpace::zObjectMeshScalarField field;
zSpace::zFnMeshScalarField fn(field);

fn.create(
    zSpace::zPoint(-10, -10, 0),
    zSpace::zPoint(10, 10, 0),
    50,
    50);

zSpace::zScalarArray values;
fn.getFieldValues(values);
```

Create a 2D vector field from a scalar mesh field:

```cpp
#include <zspace/interface.h>

zSpace::zObjectMeshScalarField scalarField;
zSpace::zFnMeshScalarField scalarFn(scalarField);

zSpace::zObjectMeshVectorField vectorField;
zSpace::zFnMeshVectorField vectorFn(vectorField);

scalarFn.create(
    zSpace::zPoint(-10, -10, 0),
    zSpace::zPoint(10, 10, 0),
    50,
    50);

vectorFn.create(
    zSpace::zPoint(-10, -10, 0),
    zSpace::zPoint(10, 10, 0),
    50,
    50);

vectorFn.createVectorFromScalarField(scalarField);
```

Create and query a 3D point scalar field:

```cpp
#include <zspace/interface.h>

zSpace::zObjectPointScalarField field;
zSpace::zFnPointScalarField fn(field);

fn.create(
    zSpace::zPoint(-10, -10, -10),
    zSpace::zPoint(10, 10, 10),
    30,
    30,
    30);

zSpace::zScalarArray values;
fn.getFieldValues(values);
```

Use these names when answering field questions:

- `zObjectMeshScalarField` with `zFnMeshScalarField` for 2D scalar fields.
- `zObjectMeshVectorField` with `zFnMeshVectorField` for 2D vector fields.
- `zObjectPointScalarField` with `zFnPointScalarField` for 3D scalar fields.
- `zObjectPointVectorField` with `zFnPointVectorField` for 3D vector fields.

Avoid `zFnMeshField<T>` and `zFnPointField<T>` in new examples unless the
question is specifically about compatibility with older code.

## Method Discovery Recipes

Find public methods on a function set:

```powershell
rg "class ZSPACE_API zFnMesh" include\zspace\zInterface\functionsets
rg "getBounds|create\\(" include\zspace\zInterface\functionsets\zFnMesh.h
```

Find implementation details:

```powershell
rg "ZSPACE_INLINE .*zFnMesh::getBounds" src\zInterface\functionsets
```

Find field usage:

```powershell
rg "zFnMeshScalarField|zFnMeshVectorField" include src tests docs agents
rg "zFnPointScalarField|zFnPointVectorField" include src tests docs agents
rg "getFieldValues|setFieldValues|getGradient|getGradients" include\zspace\zInterface\functionsets
```

Find display usage:

```powershell
rg "zDisplayScene|zDisplayMesh" include src tests docs agents
```

Find IO usage:

```powershell
rg "zIO::read|zIO::write|zCodec" include src tests docs agents
```

Find graph usage:

```powershell
rg "zFnGraph::create|zFnGraph::getEdgeData|zFnGraph::getEdgeLengths" src tests
rg "zItGraphVertex|zItGraphEdge|zItGraphHalfEdge" include src tests
```

Find migration guidance:

```powershell
rg "Migration|zObj|zObject|Display|IO" docs\architecture agents
```

## Snippet Style Rules

Use:

- `zObject*` public names.
- `zFn*` for edits and queries.
- `zIO` for file operations.
- `zDisplay*` and `zDisplayScene` for drawing.
- `zSpace::` qualifiers in standalone snippets.
- Scalar/vector-specific field function sets instead of field templates.

Avoid:

- New snippets using `zObj*` unless explaining compatibility.
- New snippets using `zFnMeshField<T>` or `zFnPointField<T>`.
- Direct use of `zGraph` or `zMesh` storage types in application snippets.
- Direct access to `.mesh`, `.graph`, `.field`, or `.particle`.
- Draw methods on objects.
- File IO through function sets.
- Host SDK types in default Core/Interface examples.
