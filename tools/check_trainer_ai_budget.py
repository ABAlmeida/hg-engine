#!/usr/bin/env python3
import sys
from pathlib import Path


capacity = 81920
reserve = 500
limit = capacity - reserve
size = Path(sys.argv[1]).stat().st_size
if size > limit:
    raise SystemExit(
        f"overlay 130 is {size} bytes; trainer AI requires <= {limit} bytes "
        f"to retain {reserve} bytes"
    )
print(f"overlay 130: {size} bytes used, {capacity - size} bytes free")
