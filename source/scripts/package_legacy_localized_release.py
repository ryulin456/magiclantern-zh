#!/usr/bin/env python3
"""Package explicitly selected 7D/1200D legacy builds, failing closed on mismatch."""
from __future__ import annotations

import argparse
import hashlib
import struct
import zipfile
from pathlib import Path, PurePosixPath

TARGETS = {
    "7D.203": ("7D_203.sym", "lua_fix.2020Dec28.7D203", 0x1F800100),
    "1200D.102": ("t5_102.sym", "Nightly.2018Aug21.1200D102", 0x00C80100),
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def archive_members(path: Path) -> dict[str, bytes]:
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        if len(names) != len(set(names)):
            raise ValueError("duplicate ZIP member names")
        for name in names:
            member = PurePosixPath(name)
            if member.is_absolute() or ".." in member.parts or "\\" in name or ":" in name:
                raise ValueError(f"unsafe ZIP member: {name!r}")
        if archive.testzip() is not None:
            raise ValueError("ZIP CRC failure")
        return {name: archive.read(name) for name in names}


def read_symbols(path: Path) -> dict[str, int]:
    symbols = {}
    for line in path.read_text(encoding="ascii").splitlines():
        if not line.strip():
            continue
        address, name = line.split(maxsplit=1)
        if name in symbols:
            raise ValueError(f"duplicate core symbol: {name}")
        symbols[name] = int(address, 16)
    return symbols


def validate_core(core: Path, sym: Path, platform: str) -> None:
    sym_name, version, restart = TARGETS[platform]
    if core.name != "autoexec.bin" or sym.name != sym_name:
        raise ValueError("wrong core or symbol filename")
    blob = core.read_bytes()
    if not 128 * 1024 <= len(blob) <= 2 * 1024 * 1024 or len(blob) % 4:
        raise ValueError(f"implausible camera core size: {len(blob)}")
    checksum = 0
    for (word,) in struct.iter_unpack("<I", blob):
        checksum ^= word
    if checksum != 0 or blob[-8:-4] != struct.pack("<I", 0xE12FFF13):
        raise ValueError("invalid camera core footer/XOR checksum")
    if version.encode("ascii") + b"\0" not in blob:
        raise ValueError("camera core embedded version mismatch")
    symbols = read_symbols(sym)
    copy_restart = symbols.get("copy_and_restart")
    if copy_restart is None:
        raise ValueError("missing core entry symbol")
    branch = struct.pack("<I", 0xEA000000 | (((copy_restart - restart - 8) >> 2) & 0xFFFFFF))
    offsets = [offset for offset in range(0, min(4096, len(blob)) - 3, 4) if blob[offset:offset+4] == branch]
    if len(offsets) != 1:
        raise ValueError("core payload entry does not match target symbol table")
    # Verify several actual instructions/data words using the exported offsets.
    raw = core.with_name("magiclantern.bin").read_bytes()
    offset = offsets[0]
    if blob[offset:offset + len(raw)] != raw:
        raise ValueError("wrapped core does not contain this target's linked payload")
    for name in ("copy_and_restart", "bmp_printf", "rbf_draw_string"):
        if name not in symbols or not restart <= symbols[name] < restart + len(raw):
            raise ValueError(f"target symbol out of payload range: {name}")


def package(original: Path, source_root: Path, output: Path, sym_name: str,
            version: str, platform: str, resources_root: Path) -> None:
    expected_sym, expected_version, _ = TARGETS[platform]
    if (sym_name, version) != (expected_sym, expected_version):
        raise ValueError("platform/symbol/version arguments do not agree")
    root = source_root.resolve(strict=True)
    build = root / "platform" / platform
    core, sym = build / "autoexec.bin", build / sym_name
    validate_core(core, sym, platform)
    out = archive_members(original)
    if "autoexec.bin" not in out or f"ML/modules/{sym_name}" not in out:
        raise ValueError("original archive does not have this target's payload layout")
    original_modules = sorted(name for name in out if name.startswith("ML/modules/") and name.endswith(".mo"))
    expected_count = 19 if platform == "7D.203" else 17
    if len(original_modules) != expected_count:
        raise ValueError(f"unexpected original module count: {len(original_modules)}")
    original_names = set(out)
    out["autoexec.bin"] = core.read_bytes()
    out[f"ML/modules/{sym_name}"] = sym.read_bytes()
    for name in original_modules:
        module = PurePosixPath(name).stem
        # Legacy builds produce these exact paths. No recursive search or fallback.
        path = root / "modules" / module / f"{module}.mo"
        blob = path.read_bytes()
        if blob[:4] != b"\x7fELF" or len(blob) < 52 or struct.unpack_from("<HH", blob, 16) != (1, 40):
            raise ValueError(f"not an ARM relocatable module: {path}")
        out[name] = blob
    resources = {
        "ML/fonts/zh_cn.lng": resources_root / "data/fonts/zh_cn.lng",
        "ML/fonts/zh_cn.bfn": resources_root / "data/fonts/zh_cn.bfn",
        "ML/doc/OFL-NotoSansSC.txt": resources_root / "doc/OFL-NotoSansSC.txt",
    }
    present = set(resources) & original_names
    if present and present != set(resources):
        raise ValueError("partially localized input archive")
    for name, path in resources.items():
        if present and out[name] != path.read_bytes():
            raise ValueError(f"localized input resource differs from selected source tree: {name}")
        out[name] = path.read_bytes()
    if set(out) != original_names | set(resources):
        raise ValueError("unexpected payload set change")
    if any(name.startswith("ML/SETTINGS/") or name.endswith(".en") for name in out):
        raise ValueError("unexpected settings or auto-enable markers")
    output.parent.mkdir(parents=True, exist_ok=True)
    # Preserve any previous package instead of silently replacing it.
    with zipfile.ZipFile(output, "x", zipfile.ZIP_DEFLATED) as archive:
        for name in sorted(out):
            archive.writestr(name, out[name])
    print(f"wrote {output} ({len(out)} entries, {sha256(output)}) version={version}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original", type=Path)
    parser.add_argument("build_root", type=Path, help="the explicit legacy source tree, not a search root")
    parser.add_argument("output", type=Path)
    parser.add_argument("sym_name")
    parser.add_argument("version")
    parser.add_argument("--platform", choices=TARGETS, required=True)
    parser.add_argument("--resources-root", type=Path,
                        help="default: selected legacy source tree's own resources")
    args = parser.parse_args()
    try:
        package(args.original, args.build_root, args.output, args.sym_name,
                args.version, args.platform, args.resources_root or args.build_root)
    except (ValueError, OSError, zipfile.BadZipFile) as error:
        parser.exit(1, f"packaging refused: {error}\n")


if __name__ == "__main__":
    main()
