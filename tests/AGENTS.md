# Tests

- Preserve meaningful behavior and edge-case coverage when consolidating suites.
- Tests must work with checked-in fixtures and declared dependencies, without
  ignored previous-source snapshots or hard-coded sibling-repository paths.
- Routine tests must not produce tracked timing reports or run large benchmarks.
- Keep benchmark commands separate. Do not duplicate the implementation as its
  own expected result; use explicit expectations, independent references or
  reviewed historical goldens.
- Do not regenerate golden results automatically when a check fails. Explain
  the behavior difference and update expectations only for an intended change.
