#!/usr/bin/env python3
"""Static release audit for the experimental 5D4.133 Chinese package."""

import argparse
import hashlib
import pathlib
import re
import struct
import subprocess
import sys
import zipfile


BFNT_HEADER = struct.Struct("<IHHIII16s")
LNG_HEADER = struct.Struct("<5I")


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate_bfnt(data):
    require(len(data) >= BFNT_HEADER.size, "BFNT header truncated")
    magic, _unknown, width, cmap_off, cmap_size, bitmap_size, name = BFNT_HEADER.unpack_from(data)
    require(magic == 0x00544E46, "BFNT magic")
    require(0 < width <= 64, "BFNT width")
    require(cmap_off >= BFNT_HEADER.size and not cmap_off % 4, "BFNT charmap offset")
    require(b"\0" in name, "BFNT name termination")
    require(not cmap_size % 4 and cmap_off <= len(data), "BFNT charmap size")
    remaining = len(data) - cmap_off
    require(cmap_size <= remaining // 2, "BFNT table bounds")
    bitmap_off = cmap_off + 2 * cmap_size
    require(bitmap_size <= len(data) - bitmap_off, "BFNT bitmap bounds")
    count = cmap_size // 4
    require(count >= 95, "BFNT ASCII coverage")
    codes = struct.unpack_from("<%dI" % count, data, cmap_off)
    offsets = struct.unpack_from("<%dI" % count, data, cmap_off + cmap_size)
    require(codes[:95] == tuple(range(0x20, 0x7F)), "BFNT ASCII ordering")
    require(offsets[0] == 0, "BFNT first glyph offset")
    for i, (code, offset) in enumerate(zip(codes, offsets)):
        if i:
            require(codes[i - 1] < code, "BFNT code ordering")
            require(offsets[i - 1] < offset, "BFNT offset ordering")
        require(not offset % 2 and offset <= bitmap_size, "BFNT glyph offset")
        require(bitmap_size - offset >= 10, "BFNT glyph header")
        glyph_width, glyph_height = struct.unpack_from("<HH", data, bitmap_off + offset)
        row_bytes = (glyph_width + 7) // 8
        require(glyph_width <= 100 and glyph_height <= 64, "BFNT glyph dimensions")
        require(row_bytes * glyph_height <= bitmap_size - offset - 10, "BFNT glyph payload")
    return set(codes)


def validate_language(data):
    require(len(data) >= LNG_HEADER.size, "language header truncated")
    magic, version, count, pool_off, pool_size = LNG_HEADER.unpack_from(data)
    require(magic == 0x485A4C4D and version == 1, "language magic/version")
    require(count <= 4096, "language entry count")
    require(pool_off == LNG_HEADER.size + count * 8, "language table offset")
    require(pool_off + pool_size == len(data), "language pool bounds")
    records = [struct.unpack_from("<II", data, LNG_HEADER.size + i * 8) for i in range(count)]
    require(all(records[i][0] < records[i + 1][0] for i in range(count - 1)),
            "language hashes must be sorted and unique")
    strings = []
    for _hash, offset in records:
        require(offset < pool_size, "language string offset")
        end = data.find(b"\0", pool_off + offset)
        require(end >= 0 and end < pool_off + pool_size, "language string termination")
        strings.append(data[pool_off + offset:end].decode("utf-8"))
    return strings


def packed_utf8(char):
    return int.from_bytes(char.encode("utf-8"), "little")


def expect_bfnt_rejections(good):
    cases = []
    for label, offset, replacement in (
        ("magic", 0, b"BAD!"),
        ("width", 6, b"\0\0"),
        ("unaligned charmap", 8, struct.pack("<I", 37)),
        ("oversized table", 12, struct.pack("<I", 0xFFFFFFFC)),
        ("oversized bitmap", 16, struct.pack("<I", 0xFFFFFFFF)),
        ("unterminated name", 20, b"X" * 16),
    ):
        broken = bytearray(good)
        broken[offset:offset + len(replacement)] = replacement
        cases.append((label, bytes(broken)))
    cases.append(("truncated", good[:BFNT_HEADER.size - 1]))
    for label, broken in cases:
        try:
            validate_bfnt(broken)
        except (ValueError, struct.error, UnicodeError):
            continue
        raise ValueError("negative BFNT test accepted: " + label)
    return len(cases)


def undefined_symbols(nm, module):
    output = subprocess.check_output([str(nm), "-u", str(module)], text=True, errors="strict")
    return {line.split()[-1] for line in output.splitlines() if line.split()}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("zip", type=pathlib.Path)
    parser.add_argument("--nm", type=pathlib.Path, required=True)
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[1]

    require(args.zip.is_file(), "release ZIP missing")
    with zipfile.ZipFile(args.zip) as archive:
        names = archive.namelist()
        require(len(names) == len(set(names)), "duplicate ZIP paths")
        require(not any(pathlib.PurePosixPath(n).is_absolute() or ".." in pathlib.PurePosixPath(n).parts
                        for n in names), "unsafe ZIP path")
        require(not any(n.lower().endswith((".fir", ".en")) for n in names), "FIR or auto-enable file present")
        require(not any("/settings/" in n.lower() for n in names), "old SETTINGS present")
        modules = {n for n in names if n.lower().endswith(".mo")}
        require(modules == {"ML/modules/bench.mo", "ML/modules/dual_iso.mo", "ML/modules/file_man.mo"},
                "unexpected module set")
        for readme in ("README.txt", "ML/README", "ML/doc/README-5D4-ZH-CN-DEVELOPER.txt"):
            text = archive.read(readme).decode("utf-8")
            require("尚未完成 5D4 实机" in text and "没有 FIR 安装器" in text, "risk notice missing")
        bfnt = archive.read("ML/fonts/zh_cn.bfn")
        language = archive.read("ML/fonts/zh_cn.lng")
        require(archive.testzip() is None, "ZIP CRC failure")

    codes = validate_bfnt(bfnt)
    strings = validate_language(language)
    missing = {ch for text in strings for ch in text if ch != "\n" and packed_utf8(ch) not in codes}
    for module in ("bench", "file_man", "dual_iso"):
        generated = (root / "modules" / module / "build" / "module_strings.h").read_text(encoding="utf-8")
        missing.update(ch for ch in generated if ord(ch) > 127 and packed_utf8(ch) not in codes)
    visible_sources = list((root / "src").glob("*.c")) + [
        root / "modules" / "bench" / "bench.c",
        root / "modules" / "file_man" / "file_man.c",
        root / "modules" / "dual_iso" / "safe_5d4.c",
    ]
    for source in visible_sources:
        text = source.read_text(encoding="utf-8")
        for literal in re.findall(r'"(?:\\.|[^"\\])*"', text):
            missing.update(ch for ch in literal if ord(ch) > 127 and packed_utf8(ch) not in codes)
    require(not missing, "missing visible glyphs: " + "".join(sorted(missing)))

    negative_count = expect_bfnt_rejections(bfnt)
    module_dir = root / "platform" / "5D4.133" / "build" / "zip" / "ML" / "modules"
    deps = {name: undefined_symbols(args.nm, module_dir / (name + ".mo"))
            for name in ("bench", "file_man", "dual_iso")}
    require(deps["bench"] <= {"menu_add", "menu_open_submenu"}, "bench unsafe dependencies")
    require(not deps["dual_iso"], "dual_iso must be a dependency-free placeholder")
    forbidden_fileman = {"FIO_CopyFile", "FIO_MoveFile", "FIO_RemoveFile", "FIO_WriteFile", "FIO_CreateFile"}
    require(not deps["file_man"] & forbidden_fileman, "file manager write dependencies")

    digest = hashlib.sha256(args.zip.read_bytes()).hexdigest()
    print("PASS: 5D4 static release audit")
    print("ZIP SHA-256:", digest)
    print("ZIP entries:", len(names))
    print("Language entries:", len(strings))
    print("Font glyphs:", len(codes))
    print("Rejected malformed BFNT cases:", negative_count)
    print("Module dependencies: bench=%d file_man=%d dual_iso=%d" %
          (len(deps["bench"]), len(deps["file_man"]), len(deps["dual_iso"])))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError, zipfile.BadZipFile) as error:
        print("FAIL:", error, file=sys.stderr)
        raise SystemExit(1)
