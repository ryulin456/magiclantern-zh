#!/usr/bin/env python3
"""Compare Magic Lantern autoexec payloads and their exported symbol tables."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


def read_symbols(path: Path) -> dict[str, int]:
    symbols: dict[str, int] = {}
    for line in path.read_text(encoding="ascii").splitlines():
        address, name = line.split(maxsplit=1)
        symbols[name] = int(address, 16)
    return symbols


def arm_branch(source: int, target: int) -> bytes:
    displacement = (target - source - 8) >> 2
    return struct.pack("<I", 0xEA000000 | (displacement & 0x00FFFFFF))


def find_payload(blob: bytes, symbols: dict[str, int], restart_start: int) -> int:
    expected = arm_branch(restart_start, symbols["copy_and_restart"])
    candidates = []
    start = 0
    while True:
        offset = blob.find(expected, start, 0x1000)
        if offset < 0:
            break
        candidates.append(offset)
        start = offset + 1
    if len(candidates) != 1:
        raise ValueError(
            f"expected one payload entry branch {expected.hex()}, found {candidates}"
        )
    return candidates[0]


def c_string(blob: bytes, offset: int, limit: int = 512) -> str:
    end = blob.find(b"\0", offset, min(offset + limit, len(blob)))
    if end < 0:
        end = min(offset + limit, len(blob))
    return blob[offset:end].decode("utf-8", errors="replace")


def analyze(label: str, binary: Path, sym_file: Path, restart_start: int) -> None:
    blob = binary.read_bytes()
    symbols = read_symbols(sym_file)
    payload = find_payload(blob, symbols, restart_start)
    print(f"[{label}]")
    print(f"autoexec_size=0x{len(blob):x} ({len(blob)})")
    print(f"payload_offset=0x{payload:x}")
    print(f"payload_size_with_wrapper_tail=0x{len(blob) - payload:x}")
    print(f"copy_and_restart=0x{symbols['copy_and_restart']:08x}")
    for name in (
        "console_show",
        "bmp_printf",
        "bfnt_draw_char",
        "bfnt_draw_char_scaled",
        "bfnt_char_get_width",
        "bfnt_char_get_width_scaled",
        "rbf_draw_string",
        "draw_beta_warning",
        "beta_warn",
        "BFNT_ALT_FONT",
        "BFNT_ALT_CHAR_CODES",
        "BFNT_ALT_BITMAP_OFFSET",
        "BFNT_ALT_BITMAP_DATA",
        "ml_started",
    ):
        if name in symbols and restart_start <= symbols[name] < 0x10000000:
            file_offset = payload + symbols[name] - restart_start
            print(f"{name}=0x{symbols[name]:08x} file=0x{file_offset:x}")
    print()


def diff_symbols(
    old_path: Path,
    new_path: Path,
    restart_start: int,
    minimum_group: int = 1,
) -> None:
    old = read_symbols(old_path)
    new = read_symbols(new_path)
    rows = sorted(
        (old_address, name, new[name] - old_address)
        for name, old_address in old.items()
        if name in new
        and restart_start <= old_address < 0x10000000
        and restart_start <= new[name] < 0x10000000
    )
    groups: list[list[tuple[int, str, int]]] = []
    for row in rows:
        if not groups or groups[-1][-1][2] != row[2]:
            groups.append([row])
        else:
            groups[-1].append(row)

    print("[symbol address-delta groups]")
    for group in groups:
        if len(group) < minimum_group:
            continue
        first, last = group[0], group[-1]
        print(
            f"old=0x{first[0]:08x}..0x{last[0]:08x} "
            f"delta={first[2]:+#x} count={len(group):4d} "
            f"symbols={first[1]}..{last[1]}"
        )
    print()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--set",
        nargs=4,
        action="append",
        metavar=("LABEL", "AUTOEXEC", "SYM", "RESTARTSTART"),
        required=True,
    )
    parser.add_argument(
        "--diff-symbols",
        nargs=3,
        metavar=("OLD_SYM", "NEW_SYM", "RESTARTSTART"),
    )
    parser.add_argument("--minimum-group", type=int, default=1)
    args = parser.parse_args()
    for label, autoexec, sym_file, restart_start in args.set:
        analyze(
            label,
            Path(autoexec),
            Path(sym_file),
            int(restart_start, 0),
        )
    if args.diff_symbols:
        old_sym, new_sym, restart_start = args.diff_symbols
        diff_symbols(
            Path(old_sym),
            Path(new_sym),
            int(restart_start, 0),
            args.minimum_group,
        )


if __name__ == "__main__":
    main()
