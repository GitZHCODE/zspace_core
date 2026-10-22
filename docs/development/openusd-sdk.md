# Core-owned OpenUSD dependency

OpenUSD is private to Core IO. Geometry and Toolsets do not require it.
No SDK binaries are copied into this repository by this change.

Enable `ZSPACE_IO_WITH_OPENUSD` to build Core IO with USD. Alice, Python and
viewer build scripts request the capability but do not accept SDK paths.

Selection order: explicit CMake `ZSPACE_OPENUSD_ROOT`, platform environment
`ZSPACE_OPENUSD_ROOT_WINDOWS_X64` / `ZSPACE_OPENUSD_ROOT_WASM32`, generic
`ZSPACE_OPENUSD_ROOT` environment, built-in `third_party/openusd/<platform>/26.08`,
then machine-local `cmake/OpenUSD.local.cmake` (see its example). The platform
names are `windows-x64` and `wasm32`. An invalid explicit override is an error.
Clear an old explicit cache override with `cmake -U ZSPACE_OPENUSD_ROOT ...`.
Resolved automatic paths are not cached. No automatic download/build occurs.

The current local configuration references the existing sibling OpenUSD-sdk
Windows/WASM installations. That configuration is ignored by Git; on another
machine supply a local configuration or environment override once in Core.

`zspace_deploy_runtime(TARGET consumer COMPONENTS io)` copies the USD runtime
closure (inspected with dumpbin), schemas and plugins. Optional
`INSTALL_DESTINATION` adds application install rules. Core shared packages
install these dependencies too and export the same helper. System DLLs remain
OS/VC-runtime prerequisites. Static Core packages additionally install the
selected SDK under share/zspace/openusd for transitive linking.

The existing 26.08 local SDK is Release x64 or Emscripten wasm32 respectively.
Use compatible toolchains/configurations; no claim of Debug/other-platform ABI
compatibility is made. Windows DLLs cannot be used for WASM.

Core WASM links SDK archives and embeds schemas. Consumers copy the generated
runtime manifest and its JS/WASM/worker/data files, never SDK archives.
SDK packaging/acquisition maintenance tools live in Core scripts/ and are
explicit tools, not part of normal Alice/Python/viewer builds.
