# Archived zModel

`zModel` was a thin object list and buffer-visibility wrapper around
`zDisplayScene`. It had no active users and duplicated the public scene
abstraction.

Use `zDisplayScene` directly for mesh and graph display configuration,
immediate drawing, and buffered drawing.
