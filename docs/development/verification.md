# Verification by change

Choose checks for the changed boundary. A routine task does not require every
native build, both WASM runtimes, the viewer build and a 500k-face benchmark.
Explicit user requirements and more specific repository instructions take priority.

| Change | Checks |
| --- | --- |
| Documentation or report index only | Review links, paths and stated commands; no compiler build |
| Native kernel/API/IO | `scripts/build.ps1 -Configuration Release`, then relevant CTest cases; run the full native suite for shared behavior |
| Standard WASM bridge/build | `wasm/scripts/build-wasm.bat`, then affected smoke scripts in `wasm/scripts/` |
| OpenUSD implementation/runtime | Build and test the OpenUSD target; do not rebuild its SDK unless SDK configuration or dependencies change |
| Performance algorithm | Correctness/parity checks first, then the relevant benchmark with stated inputs and timings |
| Test harness/fixture | Run affected tests, then the owning suite if discovery or shared helpers changed |

Native CTest commands:

```powershell
ctest --test-dir build/ninja-msvc -C Release --output-on-failure
ctest --test-dir build/ninja-msvc -C Release -R 'zspace.graph' --output-on-failure
```

The native `ninja-msvc` preset enables test and benchmark compilation. The SDK
build script disables it. Test execution is separate from compilation. Keep the
regular correctness command free of large timing workloads.

Viewer correctness and browser behavior are verified in the viewer repository;
do not introduce duplicate viewer tests here. Its current commands are documented
in that repository's `docs/development/verification.md`.

Performance executables under `tests/performance/` support kernel measurements;
graph benchmark verification modes are also registered in CTest. Historical
before/after output remains under `reports/`; new runs go to ignored output paths.
