#!/usr/bin/env python3
"""Append missing reviewed zh-CN glyphs to the compact external BFNT."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

from audit_module_text_layout import Bfnt, packed_utf8
from validate_localization import load_translations


def glyph_record(character: str, font: ImageFont.FreeTypeFont) -> bytes:
    canvas = Image.new("L", (32, 32), 0)
    draw = ImageDraw.Draw(canvas)
    # Noto Sans SC at 30 px fits a 32 px Canon cell with two safety rows.
    advance = font.getlength(character)
    x = round((32 - advance) / 2)
    draw.text((x, -2), character, font=font, fill=255)
    bitmap = bytearray()
    for y in range(32):
        for byte_x in range(4):
            value = 0
            for bit in range(8):
                if canvas.getpixel((byte_x * 8 + bit, y)) >= 80:
                    value |= 1 << (7 - bit)
            bitmap.append(value)
    if not any(bitmap):
        raise SystemExit(f"font rendered an empty glyph: {character!r}")
    return struct.pack("<5H", 32, 32, 32, 0, 0) + bytes(bitmap)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("language_file", type=Path)
    parser.add_argument("bfnt", type=Path)
    parser.add_argument("font", type=Path)
    parser.add_argument(
        "--text-source",
        type=Path,
        action="append",
        default=[],
        help="also include non-ASCII characters used by this UTF-8 source file",
    )
    parser.add_argument(
        "--text-only",
        action="store_true",
        help="ignore language_file and add glyphs only from --text-source files",
    )
    args = parser.parse_args()

    bfnt = Bfnt(args.bfnt)
    if bfnt.source_height != 32:
        raise SystemExit(f"expected a 32 px source BFNT, found {bfnt.source_height}")

    required = set()
    if not args.text_only:
        required.update(
            character
            for _, value in load_translations(args.language_file)
            for character in value
            if ord(character) > 0x7F
        )
    for source in args.text_source:
        required.update(
            character
            for character in source.read_text(encoding="utf-8")
            if ord(character) > 0x7F
        )
    existing = {
        code: bfnt.blob[
            bfnt.bitmap_start + bfnt.offsets[index] :
            bfnt.bitmap_start
            + (
                bfnt.offsets[index + 1]
                if index + 1 < len(bfnt.offsets)
                else bfnt.bitmap_size
            )
        ]
        for index, code in enumerate(bfnt.codes)
    }
    missing = sorted(
        (character for character in required if packed_utf8(character) not in existing),
        key=ord,
    )
    font = ImageFont.truetype(str(args.font), 30)
    for character in missing:
        existing[packed_utf8(character)] = glyph_record(character, font)

    rows = sorted(existing.items())
    codes = [code for code, _ in rows]
    offsets = []
    bitmap = bytearray()
    for _, record in rows:
        offsets.append(len(bitmap))
        bitmap.extend(record)

    charmap_size = 4 * len(rows)
    header = Bfnt.HEADER.pack(
        0x00544E46,
        bfnt.unknown,
        bfnt.source_height,
        Bfnt.HEADER.size,
        charmap_size,
        len(bitmap),
        bfnt.name.encode("ascii")[:15].ljust(16, b"\0"),
    )
    blob = (
        header
        + struct.pack(f"<{len(codes)}I", *codes)
        + struct.pack(f"<{len(offsets)}I", *offsets)
        + bitmap
    )
    args.bfnt.write_bytes(blob)
    print(
        f"OK: preserved {len(rows) - len(missing)} glyphs, added "
        f"{len(missing)}, total {len(rows)}"
    )


if __name__ == "__main__":
    main()
