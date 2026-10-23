# zSpace Slicer

Slicer is the example project documented for external users of the zSpace
ecosystem. It turns a tagged input mesh into layer geometry and print paths
using topology or harmonic slicing, followed by PrintSyn post-processing.

## Repositories and responsibilities

| Repository | Role in slicer | Public guide |
| --- | --- | --- |
| zspace_core | C++ geometry objects, function sets, fields and geometry IO | [Kernel foundations](kernel.md) |
| zspace_toolsets | Slicing algorithms, PrintSyn and tagged slicer USD IO | [Slicer engine](https://github.com/GitZHACODE/zspace_toolsets/blob/feature/zspace-v3-gitbook/docs/gitbook/slicer/README.md) |
| zspace_alice_webviewer | Browser input, controls, worker sessions and result display | [Browser slicer](https://github.com/GitZHACODE/zspace_alice_webviewer/blob/feature/zspace-v3-gitbook/docs/gitbook/slicer/README.md) |
| zspace_alice2 | Native desktop sketch, input preview, inspection and USD export | [Desktop slicer](https://github.com/GitZHACODE/zspace_alice2/blob/feature/zspace-v3-gitbook/docs/gitbook/slicer/README.md) |

Both applications consume geometry and toolset functionality. The slicer engine
belongs to toolsets; core provides its geometry foundation. The browser uses
compiled WASM modules, while Alice 2 links native libraries. Their UI controls,
input adapters and deployed runtimes have separate release histories.

```text
Tagged input + slicing settings
             |
             v
zspace_toolsets: topology / harmonic -> fields -> paths -> PrintSyn
             |                         |
             | uses zspace_core         +-- native libraries -> Alice 2
             +---------------------------- WASM modules -> Web Viewer
```

## Choose a workflow

Use the browser guide to run the WASM Slicing sketch with its sample or local
geometry. Use the desktop guide for the native tagged-USD workflow and print
mesh/path export. Use the engine guide when integrating slicing in C++.

The compute API requires geometry, face tags, settings and a slot anchor in a
consistent coordinate system. File adapters have their own unit and schema
contracts; a generic mesh import does not supply the complete tagged slicer
input. See the toolsets input guide before transferring data between applications.

## Source baseline

These guides were checked against the following committed source snapshots on
23 October 2026. They document source behavior; they do not certify a compatible
four-repository release or a successful runtime build.

| Repository | Source branch | Source commit before GitBook changes |
| --- | --- | --- |
| zspace_core | feature/zspace-v3-gitbook | `772c42caba7315b7d921ba4c96d38e1962b67471` |
| zspace_toolsets | slicer | `4a8329c72415917f871f1ff1bc825f1d39155fbf` |
| zspace_alice_webviewer | feature/wasm-slicing | `861e3bca15c45361aec89f3e8c8d816d538a483c` |
| zspace_alice2 | codex/task-006-print-mesh | `d8d3bd4a1e03cd19ecd16e4231b9054efde79184` |

Read [compatibility and open conflicts](compatibility.md) before combining these
versions. The browser USD build path, input schema and threading documentation
need reconciliation. No runtime parity claim is made by this documentation.

## GitBook content boundary

Each repository has its own root `gitbook-docs.yaml` pointing only to
`docs/gitbook/slicer/`. These are repository-local configurations: a relative
content directory cannot read a sibling repository. The links above connect
the four guides without guessing production GitBook URLs.

Other toolset projects, solver experiments, agent workflows, historical reports
and unrelated tutorials are outside this external documentation set. Creating
these files does not connect or publish a GitBook site.
