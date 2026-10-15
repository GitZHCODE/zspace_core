# Agent Notes

This folder is the only active place for `zspace_core` browser/WASM build work.

- Keep viewer code out of this repo. The viewer lives in `zspace_alice_webviewer`.
- Keep generated `.js`, `.wasm`, object files, caches, and temporary sysroots out of git.
- Add exported C ABI functions in `bridge/zspace_core_wasm_bridge.cpp`.
- Keep compatibility shims small and local to `bridge/`.
- The standard runtime uses `CMakeLists.txt` through `scripts/build-wasm.bat`.
  Core/interface/IO sources are discovered automatically; update the CMake
  target when changing source exclusions, flags, or exported runtime methods.
- Treat zspace toolsets as a future separate repo; do not add toolset-specific build logic here yet.

Before committing wasm build changes, run:

```bat
wasm\scripts\build-wasm.bat
```
