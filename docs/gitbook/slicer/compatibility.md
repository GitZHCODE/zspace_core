# Compatibility and open conflicts

This register records source and documentation conflicts found while preparing
the slicer GitBook guides on 23 October 2026. Baselines are listed in the
[project overview](README.md). No implementation fixes or runtime parity tests
were performed as part of that preparation. Owners below refer to repository
responsibilities, not assigned individuals.

## SLICER-001: browser USD build target is missing

**Status:** confirmed source mismatch. **Owners:** toolsets and web viewer.

The web viewer's combined builder requires
`wasm/scripts/build-slicing-usd-wasm.ps1` in toolsets and deploys
`slicing-usd/zspace_slicing_usd.js` plus `.wasm`. Neither that script nor the
dedicated importer target is present in toolsets commit `4a8329c7`. The viewer
loader calls `zspace_slicing_usd_read` and its result/error exports.

**Effect:** these snapshots cannot reproduce the documented combined
`build-slicing.bat` workflow. The compute-only builder exists separately.

**Resolution needed:** restore/version the importer implementation and builder,
or deliberately migrate the viewer to another supported importer ABI. Verify a
fresh build of both pairs and the viewer's focused slicing/USD tests. Existing
ignored binaries are not proof of a reproducible source build.

Evidence: [combined builder](https://github.com/GitZHACODE/zspace_alice_webviewer/blob/861e3bca15c45361aec89f3e8c8d816d538a483c/scripts/build-viewer.ps1),
[importer calls](https://github.com/GitZHACODE/zspace_alice_webviewer/blob/861e3bca15c45361aec89f3e8c8d816d538a483c/src/io/slicingInput.ts),
[toolsets WASM tree](https://github.com/GitZHACODE/zspace_toolsets/tree/4a8329c72415917f871f1ff1bc825f1d39155fbf/wasm).

## SLICER-002: tagged USD schema descriptions disagree

**Status:** confirmed documentation/native-source mismatch; deployed browser
importer behavior unverified. **Owners:** toolsets, desktop and web viewer.

The older browser guide describes `/block`, `/bracing`, `/slotpoint`, face
primvars and root-layer setting dictionaries. Current native code defaults to
`/AG_block`, `/Slicer_bracing`, `/Slicer_slot`, with tag arrays in `/Slicer_data`
and namespaced settings in `/Slicer_setting`. It also expects linear
BasisCurves for bracing. These are different contracts.

**Effect:** following the older guide does not establish that an input is valid
for the current native reader. The missing importer source prevents a source
comparison of the deployed browser USD implementation.

**Resolution needed:** choose a versioned canonical schema, specify any supported
legacy conversion, and test the same fixture through native and browser readers.
Include face-tag order, width-group counts, bracing curves and slot geometry.

Evidence: [older browser input guide](https://github.com/GitZHACODE/zspace_alice_webviewer/blob/861e3bca15c45361aec89f3e8c8d816d538a483c/docs/development/slicing.md),
[current native reader](https://github.com/GitZHACODE/zspace_toolsets/blob/4a8329c72415917f871f1ff1bc825f1d39155fbf/src/zToolsets/fabrication/slicing/utils/IO.cpp),
[desktop fixture](https://github.com/GitZHACODE/zspace_alice2/blob/d8d3bd4a1e03cd19ecd16e4231b9054efde79184/alice2/verification/fixtures/slicing-block.usda).

## SLICER-003: unit and up-axis descriptions disagree

**Status:** confirmed documentation/native-source mismatch; browser parity
unverified. **Owners:** toolsets, desktop and web viewer.

The browser guide says USD coordinates remain in authored stage units and Y-up
geometry is rotated to Z-up. Current native slicing IO applies transforms and
normalizes geometry and dimensional settings using `metersPerUnit * 1000`.
It records `UpAxis` but has no corresponding automatic axis-rotation step.
Generic core mesh IO has its own object-space contract.

**Effect:** assuming that every importer uses the same coordinate conversion can
change dimensions or orientation, or apply a conversion twice.

**Resolution needed:** define the canonical compute units and orientation, then
test millimetre/metre stages, transformed geometry and both Y-up/Z-up inputs
through each supported adapter. Verify widths, layer spacing, seam lengths,
normals and export round trips; ratios and angles must remain unchanged.

Evidence: [browser unit description](https://github.com/GitZHACODE/zspace_alice_webviewer/blob/861e3bca15c45361aec89f3e8c8d816d538a483c/docs/development/slicing.md),
[native conversions](https://github.com/GitZHACODE/zspace_toolsets/blob/4a8329c72415917f871f1ff1bc825f1d39155fbf/src/zToolsets/fabrication/slicing/utils/IO.cpp),
[generic core IO](https://github.com/GitZHACODE/zspace_core/blob/772c42caba7315b7d921ba4c96d38e1962b67471/docs/architecture/openusd-io.md).

## SLICER-004: browser threading documentation is ahead of the UI

**Status:** confirmed source/documentation mismatch. **Owners:** toolsets and web viewer.

The tagged-slicing architecture guide says OpenMP is disabled in the browser
build. Current `wasm/slicing/CMakeLists.txt` compiles and links pthreads/OpenMP,
and the newer WASM README describes `useParallel` plus **Compute** and
**Parallel compute** actions. The selected web sketch has only **Compute slices**;
its `SlicingInput` type has no declared `useParallel` field.

**Effect:** documented controls and threading requirements differ across these
snapshots. Do not claim a parallel UI or a serial, isolation-free compute binary
without checking the application/runtime combination.

**Resolution needed:** align the input type, UI and runtime contract, update the
older architecture guide, and check serial/parallel numerical results, cancellation
and worker cleanup. Confirm production COOP/COEP headers. Measure performance
separately before claiming a speedup.

Evidence: [older architecture guide](https://github.com/GitZHACODE/zspace_toolsets/blob/4a8329c72415917f871f1ff1bc825f1d39155fbf/docs/architecture/tagged-slicing.md),
[current WASM README](https://github.com/GitZHACODE/zspace_toolsets/blob/4a8329c72415917f871f1ff1bc825f1d39155fbf/wasm/README.md),
[compute target](https://github.com/GitZHACODE/zspace_toolsets/blob/4a8329c72415917f871f1ff1bc825f1d39155fbf/wasm/slicing/CMakeLists.txt),
[web actions](https://github.com/GitZHACODE/zspace_alice_webviewer/blob/861e3bca15c45361aec89f3e8c8d816d538a483c/sketches/sketch_sliceMesh.js),
[web input type](https://github.com/GitZHACODE/zspace_alice_webviewer/blob/861e3bca15c45361aec89f3e8c8d816d538a483c/src/kernels/slicingWasmSession.ts).

## SLICER-005: numeric-reference source paths are obsolete

**Status:** confirmed documentation-link mismatch. **Owner:** toolsets.

`docs/SLICER_NUMERIC_REFERENCE.md` points to the old
`cpp/headers/zToolsets/ts3dp/...` and `cpp/source/zToolsets/ts3dp/...` layout.
The referenced `Utils.h` and `zPrintSyn.cpp` paths do not exist at the selected
commit; maintained code is under `include/zspace/zToolsets/fabrication/slicing/`
and `src/zToolsets/fabrication/slicing/`.

**Effect:** the historical snapshot cannot serve as a current linked parameter
reference. This finding establishes broken locations, not that every listed
numeric value is wrong.

**Resolution needed:** reconcile the table with current declarations and
implementation, update source paths and line references, and distinguish library
defaults from application presets. The external guide currently links current
source rather than importing this historical table.

Evidence: [numeric reference](https://github.com/GitZHACODE/zspace_toolsets/blob/4a8329c72415917f871f1ff1bc825f1d39155fbf/docs/SLICER_NUMERIC_REFERENCE.md),
[current settings header](https://github.com/GitZHACODE/zspace_toolsets/blob/4a8329c72415917f871f1ff1bc825f1d39155fbf/include/zspace/zToolsets/fabrication/slicing/utils/Utils.h).

## Verification item: establish a compatible release set

**Status:** pending verification, not a confirmed runtime defect.
**Owners:** all four repositories.

The guides pin independent committed source baselines. No build or end-to-end
comparison was run for the documentation change. A matching GitBook branch name
does not make those baselines a tested application release.

Before advertising one supported release, record the four commits, SDK versions
and JS/WASM artifact hashes. Run the existing native slicing tests and focused
browser slicing/USD tests, then compare both applications on the same input and
settings. Check topology/harmonic capabilities, layer geometry, widths/heights,
PrintSyn paths and desktop export. Record measurements separately from estimates.
