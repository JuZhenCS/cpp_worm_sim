from __future__ import annotations

import ctypes
import math
import os
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]


def find_shared_library() -> Path:
    candidates = [
        PROJECT_ROOT / "build" / "worm_brain_shared.dll",
        PROJECT_ROOT / "build" / "libworm_brain_shared.dll",
        PROJECT_ROOT / "build" / "Debug" / "worm_brain_shared.dll",
        PROJECT_ROOT / "build" / "Debug" / "libworm_brain_shared.dll",
        PROJECT_ROOT / "build" / "Release" / "worm_brain_shared.dll",
        PROJECT_ROOT / "build" / "Release" / "libworm_brain_shared.dll",
        PROJECT_ROOT / "cmake-build-debug" / "worm_brain_shared.dll",
        PROJECT_ROOT / "cmake-build-debug" / "libworm_brain_shared.dll",
        PROJECT_ROOT / "cmake-build-release" / "worm_brain_shared.dll",
        PROJECT_ROOT / "cmake-build-release" / "libworm_brain_shared.dll",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError("worm_brain_shared DLL not found. Build it first with: cmake --build build --target worm_brain_shared")


def dll_search_dirs(shared_library: Path) -> list[Path]:
    dirs = [shared_library.parent]
    cache = PROJECT_ROOT / "build" / "CMakeCache.txt"
    if cache.exists():
        for line in cache.read_text(encoding="utf-8", errors="ignore").splitlines():
            if line.startswith("CMAKE_CXX_COMPILER:FILEPATH="):
                compiler = Path(line.split("=", 1)[1])
                dirs.append(compiler.parent)
    return list(dict.fromkeys(dirs))


def main() -> None:
    shared_library = find_shared_library()
    dll_dir_handles = []
    if hasattr(os, "add_dll_directory"):
        for directory in dll_search_dirs(shared_library):
            if directory.exists():
                dll_dir_handles.append(os.add_dll_directory(str(directory)))

    lib = ctypes.CDLL(str(shared_library))
    lib.worm_brain_init.argtypes = [ctypes.c_char_p]
    lib.worm_brain_init.restype = ctypes.c_int32
    lib.worm_brain_reset.argtypes = []
    lib.worm_brain_reset.restype = None
    lib.simulate.argtypes = [ctypes.c_int32]
    lib.simulate.restype = ctypes.POINTER(ctypes.c_float)
    lib.worm_brain_muscle_count.argtypes = []
    lib.worm_brain_muscle_count.restype = ctypes.c_int32
    lib.worm_brain_last_error.argtypes = []
    lib.worm_brain_last_error.restype = ctypes.c_char_p
    lib.worm_brain_shutdown.argtypes = []
    lib.worm_brain_shutdown.restype = None

    rc = lib.worm_brain_init(str(PROJECT_ROOT).encode("utf-8"))
    if rc != 0:
        raise RuntimeError(lib.worm_brain_last_error().decode("utf-8"))

    try:
        muscle_count = lib.worm_brain_muscle_count()
        if muscle_count != 96:
            raise RuntimeError(f"unexpected muscle count: {muscle_count}")

        for call_index in range(3):
            ptr = lib.simulate(10)
            if not ptr:
                raise RuntimeError(lib.worm_brain_last_error().decode("utf-8"))
            values = [float(ptr[i]) for i in range(muscle_count)]
            if len(values) != 96 or not all(math.isfinite(value) for value in values):
                raise RuntimeError("simulate returned invalid muscle output")
            print(f"call={call_index} count={len(values)} min={min(values):.6g} max={max(values):.6g} first8={values[:8]}")
    finally:
        lib.worm_brain_shutdown()


if __name__ == "__main__":
    main()