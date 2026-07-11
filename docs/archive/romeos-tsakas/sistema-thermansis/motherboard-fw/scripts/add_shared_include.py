"""Προσθέτει ../shared/include στο CPPPATH (Unicode paths στο Windows)."""
Import("env")

from pathlib import Path

shared_include = (Path(env["PROJECT_DIR"]) / ".." / "shared" / "include").resolve()
if shared_include.is_dir():
    env.Append(CPPPATH=[str(shared_include)])
    print(f"[build] shared include: {shared_include}")
else:
    print(f"[build] WARN: missing {shared_include}")
