# Evidence index

Reports are historical measurements and implementation records, not the current
build configuration. Use the root README, `wasm/README.md` and the agent workflow
for active instructions.

| Topic | Start here |
| --- | --- |
| Mesh storage/topology | [Mesh topology audit](mesh-topology-audit.html) |
| Smoothing | [Smooth mesh optimization](smooth-mesh-optimization.html) |
| Subdivision | [Subdivision optimization](subdivision-optimization.html) |
| Normals/render export | [Render export optimization](render-export-optimization.html) |
| Graph storage/topology | [Graph topology audit](graph-topology-audit.html) |
| Graph optimization | [Graph optimization](graph-optimization.html) |
| Graph construction | [Construction optimization](graph-construction-optimization.html) |
| 2D & 3D Fields | [Field optimization](field-optimization.md) |
| Typed sketch buffers | [Typed buffer audit](sketch-typed-audit.html) |
| Viewer boundaries | [Boundary table optimization](viewer-boundary-table-optimization.html) |
| Viewer reconstruction | [Reconstruction optimization](viewer-reconstruction-optimization.html) |
| Viewer bounds | [Surface bounds optimization](viewer-surface-bounds-optimization.html) |

Similarly named JSON, CSV and images contain supporting evidence. Older
`before`, `first`, and `initial` results describe intermediate runs; they are not
current baselines. Historical report generators stay at their recorded paths.

Viewer source organization and its implementation roadmap are documented in
the viewer repository. New viewer benchmarks are owned there and write to its
ignored `build/benchmarks/` folder instead of updating these reports.
