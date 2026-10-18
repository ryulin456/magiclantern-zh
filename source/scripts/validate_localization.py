#!/usr/bin/env python3
"""Static checks for the zh-CN source resources and compact BFNT font."""

from __future__ import annotations

import argparse
import re
import struct
from pathlib import Path


MODULE_ROW_RE = re.compile(r'^\s*\{ "([^"]+)", "(.*)", "(.*)" \},$')
ZH_HEADER = struct.Struct("<4sIIII")
ZH_ENTRY = struct.Struct("<II")


def packed_utf8(character: str) -> int:
    return int.from_bytes(character.encode("utf-8"), "little")


def load_font_codes(path: Path) -> set[int]:
    blob = path.read_bytes()
    if len(blob) < 36 or blob[:4] != b"FNT\0":
        raise SystemExit(f"invalid BFNT header: {path}")
    charmap_offset, charmap_size = struct.unpack_from("<II", blob, 8)
    if charmap_size % 4 or charmap_offset + charmap_size > len(blob):
        raise SystemExit(f"invalid BFNT character map: {path}")
    return set(
        struct.unpack_from(
            f"<{charmap_size // 4}I", blob, charmap_offset
        )
    )


def load_translations(path: Path) -> list[tuple[int, str]]:
    blob = path.read_bytes()
    if len(blob) < ZH_HEADER.size:
        raise SystemExit(f"translation file is too small: {path}")
    magic, version, count, pool_offset, pool_size = ZH_HEADER.unpack_from(blob)
    expected_pool_offset = ZH_HEADER.size + count * ZH_ENTRY.size
    if magic != b"MLZH" or version != 1:
        raise SystemExit(f"invalid translation header: {path}")
    if pool_offset != expected_pool_offset or pool_offset + pool_size != len(blob):
        raise SystemExit(f"invalid translation bounds: {path}")

    translations = []
    previous_hash = -1
    for index in range(count):
        hash_value, offset = ZH_ENTRY.unpack_from(
            blob, ZH_HEADER.size + index * ZH_ENTRY.size
        )
        if hash_value <= previous_hash or offset >= pool_size:
            raise SystemExit(f"invalid translation entry {index}: {path}")
        end = blob.find(b"\0", pool_offset + offset, pool_offset + pool_size)
        if end < 0:
            raise SystemExit(f"unterminated translation entry {index}: {path}")
        value = blob[pool_offset + offset : end].decode("utf-8")
        translations.append((hash_value, value))
        previous_hash = hash_value
    return translations


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source_root", type=Path)
    parser.add_argument("--map", type=Path)
    parser.add_argument("--dynamic-baseline-map", type=Path)
    parser.add_argument(
        "--reservation-mode",
        choices=("growth-cap", "dynamic-bss", "fixed-end"),
        default="growth-cap",
        help=(
            "growth-cap limits growth relative to the baseline; dynamic-bss "
            "is for boot code that reserves exactly through _bss_end; "
            "fixed-end checks the linked image against a fixed reservation end"
        ),
    )
    parser.add_argument("--max-reservation-growth", type=int, default=8192)
    parser.add_argument("--fixed-reservation-end", type=lambda value: int(value, 0))
    parser.add_argument("--autoexec", type=Path)
    parser.add_argument("--expected-version")
    args = parser.parse_args()
    root = args.source_root

    translation_path = root / "data" / "fonts" / "zh_cn.lng"
    module_path = root / "src" / "zh_cn_modules.h"
    checked_paths = [
        module_path,
        root / "src" / "menuhelp.c",
        root / "src" / "module.c",
        root / "src" / "gps.c",
        root / "modules" / "arkanoid" / "arkanoid.c",
        root / "modules" / "vid_af" / "vid_af.c",
    ]

    translations = load_translations(translation_path)
    if len(translations) != 2052:
        raise SystemExit(f"expected 2052 translations, found {len(translations)}")
    hashes = [row[0] for row in translations]
    if hashes != sorted(hashes) or len(hashes) != len(set(hashes)):
        raise SystemExit("translation hashes are not unique and sorted")

    modules = [
        match.groups()
        for line in module_path.read_text(encoding="utf-8").splitlines()
        if (match := MODULE_ROW_RE.match(line))
    ]
    if len(modules) != 19:
        raise SystemExit(f"expected 19 module records, found {len(modules)}")

    text = "".join(value for _, value in translations)
    text += "".join(path.read_text(encoding="utf-8") for path in checked_paths)
    if "\ufffd" in text:
        raise SystemExit("Unicode replacement character found")
    if "Codex" in text or "codex" in text:
        raise SystemExit("Codex branding found")

    codes = load_font_codes(root / "data" / "fonts" / "zh_cn.bfn")
    missing = sorted(
        {
            character
            for character in text
            if ord(character) > 0x7F and packed_utf8(character) not in codes
        },
        key=ord,
    )
    if missing:
        codepoints = ", ".join(f"U+{ord(character):04X}" for character in missing)
        raise SystemExit(
            "font is missing characters: " + "".join(missing) + f" ({codepoints})"
        )

    if args.dynamic_baseline_map and not args.map:
        raise SystemExit("--dynamic-baseline-map requires --map")
    if args.map:
        map_text = args.map.read_text(encoding="utf-8", errors="replace")
        match = re.search(r"^\.bss\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)", map_text, re.M)
        if not match:
            raise SystemExit(f"could not find .bss bounds in {args.map}")
        bss_end = int(match.group(1), 16) + int(match.group(2), 16)
        if args.dynamic_baseline_map:
            baseline_text = args.dynamic_baseline_map.read_text(
                encoding="utf-8", errors="replace"
            )
            baseline_match = re.search(
                r"^\.bss\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)",
                baseline_text,
                re.M,
            )
            if not baseline_match:
                raise SystemExit(
                    f"could not find baseline .bss bounds in {args.dynamic_baseline_map}"
                )
            baseline_end = int(baseline_match.group(1), 16) + int(
                baseline_match.group(2), 16
            )
            reserved_growth = bss_end - baseline_end
            if (
                args.reservation_mode == "growth-cap"
                and reserved_growth > args.max_reservation_growth
            ):
                raise SystemExit(
                    f"dynamic boot reservation grew by {reserved_growth} bytes; "
                    f"limit is {args.max_reservation_growth}"
                )
            if args.reservation_mode == "fixed-end":
                if args.fixed_reservation_end is None:
                    raise SystemExit(
                        "--reservation-mode fixed-end requires "
                        "--fixed-reservation-end"
                    )
                if bss_end > args.fixed_reservation_end:
                    raise SystemExit(
                        f"linked image ends at 0x{bss_end:x}, beyond fixed "
                        f"reservation end 0x{args.fixed_reservation_end:x}"
                    )
                reservation_headroom = args.fixed_reservation_end - bss_end
            elif args.fixed_reservation_end is not None:
                raise SystemExit(
                    "--fixed-reservation-end requires --reservation-mode fixed-end"
                )
        else:
            if args.reservation_mode == "fixed-end":
                if args.fixed_reservation_end is None:
                    raise SystemExit("--reservation-mode fixed-end requires --fixed-reservation-end")
                if bss_end > args.fixed_reservation_end:
                    raise SystemExit(
                        f"linked image ends at 0x{bss_end:x}, beyond fixed "
                        f"reservation end 0x{args.fixed_reservation_end:x}"
                    )
                reservation_headroom = args.fixed_reservation_end - bss_end
            else:
                safe_limit = 0x4DC000  # keep at least 16 KiB below the 6D pool at 0x4E0000
                if bss_end > safe_limit:
                    raise SystemExit(
                        f"6D memory image is unsafe: .bss ends at 0x{bss_end:x}, "
                        f"limit is 0x{safe_limit:x}"
                    )

    if bool(args.autoexec) != bool(args.expected_version):
        raise SystemExit("--autoexec and --expected-version must be used together")
    if args.autoexec:
        autoexec = args.autoexec.read_bytes()
        expected = args.expected_version.encode("ascii")
        if expected not in autoexec:
            raise SystemExit(
                f"expected upstream version {args.expected_version!r} "
                f"not found in {args.autoexec}"
            )
        if re.match(r"^20\d{2}-\d{2}-\d{2}", args.expected_version):
            suffix = args.expected_version[10:]
            versions = {
                match.group().decode("ascii")
                for match in re.finditer(
                    rb"20\d{2}-\d{2}-\d{2}" + re.escape(suffix.encode("ascii")),
                    autoexec,
                )
            }
        else:
            versions = {args.expected_version} if expected in autoexec else set()
        if versions != {args.expected_version}:
            raise SystemExit(
                "unexpected firmware version strings in autoexec: "
                + ", ".join(sorted(versions))
            )

    print(
        f"OK: {len(translations)} translations, {len(modules)} modules, "
        f"{len(codes)} BFNT glyphs, no missing characters"
        + (f", .bss ends at 0x{bss_end:x}" if args.map else "")
        + (
            f", reservation growth {reserved_growth} bytes"
            if args.dynamic_baseline_map
            else ""
        )
        + (
            f", fixed reservation headroom {reservation_headroom} bytes"
            if args.map
            and args.dynamic_baseline_map
            and args.reservation_mode == "fixed-end"
            else ""
        )
        + (f", version {args.expected_version}" if args.autoexec else "")
    )


if __name__ == "__main__":
    main()
