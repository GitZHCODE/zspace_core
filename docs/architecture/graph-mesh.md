# Graph ribbon mesh

`zFnMesh(output).graphMesh(graph, offset)` replaces the attached output mesh with
a quad ribbon. `offset` is a finite positive distance on **each side** of each
member's XY projection: the nominal total width is `2 * offset`. Each graph
station retains its elevation, so a member can slope in Z. This is a horizontal
ribbon convention, not an arbitrary-plane sweep or a solid extrusion.

The ribbon keeps one centre vertex per incident graph vertex and shares mitre
corners between adjoining members. Each edge produces two quads, split at the
centreline. Chains, closed loops, branches and separate components use the same
construction. Isolated graph vertices are omitted. Terminal cross-sections end
at the graph endpoints; no thickness or longitudinal extension is added.

The method uses graph positions/connectivity and face lists without requiring
halfedge traversal. It validates positions, member indices, outgoing directions
and projected face turns before replacing the output. Vertical/collapsed XY
members and offsets which consume a short member or fold a face are rejected.
Global intersections between unrelated members are not resolved; input should
describe a noncrossing layout in XY.

The source graph is unchanged. Errors leave the previous output geometry intact.
Native coverage lives in `tests/smoke/zspace_smoke.cpp`.

The standard WASM bridge exposes `zspace_mesh_graph_from_buffers` with Float32
positions, Uint32 edge endpoints and a scalar offset. It publishes through the
existing mesh render buffers. Alice's adapter copies the result into owned typed
arrays and releases temporary upload buffers; this path uses no JSON transfer.
