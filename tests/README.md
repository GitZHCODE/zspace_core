# Core verification

- `smoke/`: native behavioral tests, small fixtures and graph golden results.
- `headers/`: public-header compilation checks.
- `performance/`: mesh and graph benchmark programs and optional diagnostic tools.
- `mesh-iterator-runtime-parity.mjs`: explicit comparison of two supplied WASM
  runtimes; it is not part of normal native CTest execution.

See [verification commands](../docs/development/verification.md). Viewer tests
and benchmarks belong in `zspace_alice_webviewer`; former duplicate comparison
scripts here were consolidated there. Historical measurements remain in reports.
