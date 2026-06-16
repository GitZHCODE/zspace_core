# Archived Spatial Bin

`zObjSpatialBin` and `zFnSpatialBin` had no active consumers outside their own
implementation and the old zApp aggregate include. Their draw methods were
empty, and the subsystem exposed point-cloud storage directly.

The files are preserved here for reference. If spatial indexing is needed
again, restore it as a focused spatial-index module rather than as a
point-cloud subclass.
