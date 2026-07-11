"""PlatformIO pre-script: τρέχει convert_preview_album.py με το system Python (Pillow)."""
Import("env")

import os
import subprocess
import sys
from pathlib import Path

PROJECT_DIR = Path(env["PROJECT_DIR"])
converter = PROJECT_DIR / "scripts" / "convert_preview_album.py"

# Προτίμηση του Python που έχει Pillow (συνήθως το system python).
def _has_pillow(cmd: list[str]) -> bool:
    try:
        r = subprocess.run(
            cmd + ["-c", "import PIL"],
            capture_output=True,
            timeout=20,
        )
        return r.returncode == 0
    except (OSError, subprocess.TimeoutExpired):
        return False


candidates: list[list[str]] = []
if os.environ.get("ROMEOS_PYTHON"):
    candidates.append([os.environ["ROMEOS_PYTHON"]])
candidates.extend(
    [
        [r"C:\Python314\python.exe"],
        ["py", "-3"],
        ["python3"],
        ["python"],
    ]
)

python_cmd = None
for cmd in candidates:
    if _has_pillow(cmd):
        python_cmd = cmd
        break

if python_cmd is None:
    print(
        "[preview_album] ERROR: pip install pillow  (then: py -3 -m pip install pillow)",
        file=sys.stderr,
    )
    raise SystemExit(1)

result = subprocess.run(python_cmd + [str(converter)], cwd=str(PROJECT_DIR))
if result.returncode != 0:
    raise SystemExit(result.returncode)
