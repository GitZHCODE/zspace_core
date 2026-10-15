# zspace_kernel_agent

Use this guide for zSpace geometry kernel work, including C++ core updates,
WASM bridge exports, Emscripten build changes, and generated JavaScript/WASM
runtime artifacts.

## Scope

Owns:

- `include/` and `src/` geometry kernel APIs.
- `tests/` coverage for core/interface/io behavior.
- `wasm/bridge/` C++ bridge code.
- `wasm/scripts/` Emscripten setup and build scripts.
- `wasm/README.md` and `wasm/AGENTS.md`.

Does not own:

- Alice viewer UI or TypeScript app behavior.
- Live viewer sketches and operators.
- zspace toolsets; those belong in a future separate repo.

## Rules

- Keep generated outputs out of git: `build/`, `wasm/build/`, `wasm/tmp/`,
  `wasm/cache/`, and generated files in `wasm/out/`.
- Add browser-facing functions in `wasm/bridge/zspace_core_wasm_bridge.cpp`.
- The standard WASM build is defined in `wasm/CMakeLists.txt`, with
  `wasm/scripts/build-wasm.bat` as its entrypoint. Core/interface/IO C++ sources
  are discovered automatically; update the target for source exclusions,
  flags, or exported runtime methods.
- Keep Emscripten compatibility shims local to `wasm/bridge/`.
- Preserve the existing C++ API direction: objects own data, function sets own
  operations, IO stays in `zIO`, display stays in `zDisplay`.

## Verification

Choose checks for the changed boundary using
[verification by change](../docs/development/verification.md). Documentation-only
work does not require compiler builds. Large performance comparisons are opt-in;
do not run viewer benchmarks or rewrite tracked reports for every kernel task.

For C++ kernel changes:

```powershell
.\scripts\build.ps1 -Configuration Release
ctest --test-dir build\ninja-msvc -C Release --output-on-failure
```

For WASM bridge changes:

```bat
wasm\scripts\build-wasm.bat
```
