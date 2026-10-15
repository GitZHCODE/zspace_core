# Repository workflow

This repository owns the C++ geometry kernel and its WASM build. The Alice viewer
is a separate repository; change it only when the request includes viewer work.

## Start with the requested scope

- Inspect `git status` and preserve unrelated or concurrent changes.
- Read the root README and the instructions for the area being edited. For kernel
  work use `agents/zspace_kernel_agent.md`; for WASM also read `wasm/AGENTS.md`.
- Follow [verification guidance](docs/development/verification.md). Do not read
  every report, rebuild every runtime, or run every benchmark for each request.
- Keep public APIs, compatibility modes, sketches and UI behavior intact unless
  the user asks to change them. Prefer small edits in existing responsibility
  folders over a general redesign.
- Do not commit or push unless requested. Do not update roadmap checkboxes until
  the relevant implementation and verification pass.

## Ownership and output

- `include/` is the public API; `src/` holds implementation; `tests/` holds
  repeatable correctness checks and fixtures.
- `scripts/` and `wasm/scripts/` hold maintained tools, not generated results.
- `reports/` contains historical evidence. Follow `reports/AGENTS.md`; routine
  tests must not rewrite tracked reports.
- `legacy/` and `archive/` are reference material, not maintained build inputs.
  Do not remove them or vendored dependencies without checking compatibility users.
- Generated builds, compiler caches and temporary comparisons remain ignored.

## Finish

Report changed files, the checks run and their results, and checks omitted because
they do not apply or could not run. Distinguish measurements from estimates.
