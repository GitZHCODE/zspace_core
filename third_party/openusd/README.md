# Optional built-in OpenUSD SDKs

No SDK binaries are included here. Current builds use Core-local configuration.

A future built-in installation may be placed at `windows-x64/26.08/` or
`wasm32/26.08/`, preserving the complete SDK layout and licenses. Core will
select it automatically unless an explicit CMake/environment override is set.
See ../../docs/development/openusd-sdk.md for selection and deployment rules.
Do not place native and WASM libraries in the same SDK directory.
