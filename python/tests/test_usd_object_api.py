"""Run with the USD-enabled Core package on PYTHONPATH."""
import gc
import tempfile
from pathlib import Path
from zspace.core import USDdoc

with tempfile.TemporaryDirectory() as folder:
    source = Path(folder) / "input.usda"
    source.write_text('#usda 1.0\ndef Scope "Settings" {}\n')
    doc = USDdoc()
    doc.open(str(source))
    prim = doc.getPrimAtPath("/Settings")
    assert prim.isValid()
    width = prim.createAttribute("width", "float")
    width.set(2.5)
    assert width.get() == 2.5
    points = prim.createAttribute("points", "point3f[]")
    points.set([1, 2, 3, 4, 5, 6])
    assert points.get() == [1, 2, 3, 4, 5, 6]
    try:
        width.set("invalid")
        raise AssertionError("type mismatch accepted")
    except (TypeError, RuntimeError):
        pass
    output = Path(folder) / "output.usda"
    doc.save(str(output))
    doc.open(str(output))
    assert not width.isValid()
    width = doc.getPrimAtPath("/Settings").getAttribute("width")
    assert width.get() == 2.5
    del doc
    gc.collect()
    assert not width.isValid()
print("Python USD object API checks passed")
