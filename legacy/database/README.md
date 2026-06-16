# Archived SQLite Support

This folder preserves the former `zDatabase` wrapper and bundled SQLite sources.
They are not part of the active zSpace build or public API.

Original locations:

- `include/zspace/zCore/data/zDatabase.h`
- `src/zCore/data/zDatabase.cpp`
- `third_party/depends/SQLITE`

If database support is needed again, restore it as a separate optional library
rather than adding SQLite back to `zSpace_Core`.
