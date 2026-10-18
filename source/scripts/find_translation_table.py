#!/usr/bin/env python3
"""Find English-to-Chinese pointer pairs in a localized ML autoexec payload."""

from __future__ import annotations

import argparse
import collections
import re
import struct
from pathlib import Path

from analyze_ml_binaries import find_payload, read_symbols


CJK_RE = re.compile(r"[\u3400-\u4dbf\u4e00-\u9fff]")
PRINTABLE_RE = re.compile(r"^[^\x00-\x08\x0b\x0c\x0e-\x1f\x7f]+$")


def utf8_strings(blob: bytes, minimum: int = 2):
    for match in re.finditer(rb"[^\x00]{%d,}\x00" % minimum, blob):
        raw = match.group()[:-1]
        try:
            value = raw.decode("utf-8")
        except UnicodeDecodeError:
            continue
        if PRINTABLE_RE.match(value):
            yield match.start(), value


def resolve_string(
    blob: bytes,
    address: int,
    payload: int,
    restart_start: int,
) -> str | None:
    offset = payload + address - restart_start
    if not (0 <= offset < len(blob)):
        return None
    end = blob.find(b"\0", offset, min(offset + 1024, len(blob)))
    if end < 0:
        return None
    try:
        value = blob[offset:end].decode("utf-8")
    except UnicodeDecodeError:
        return None
    return value if PRINTABLE_RE.match(value) else None


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("autoexec", type=Path)
    parser.add_argument("sym", type=Path)
    parser.add_argument("restart_start", type=lambda value: int(value, 0))
    parser.add_argument("--limit", type=int, default=100)
    parser.add_argument("--dump", type=lambda value: int(value, 0))
    parser.add_argument("--dump-count", type=int, default=64)
    args = parser.parse_args()

    blob = args.autoexec.read_bytes()
    symbols = read_symbols(args.sym)
    payload = find_payload(blob, symbols, args.restart_start)
    cjk = {
        args.restart_start + offset - payload: (offset, value)
        for offset, value in utf8_strings(blob)
        if CJK_RE.search(value)
    }

    refs: list[tuple[int, int]] = []
    for offset in range(payload, len(blob) - 3, 4):
        address = struct.unpack_from("<I", blob, offset)[0]
        if address in cjk:
            refs.append((offset, address))

    pages = collections.Counter(offset >> 12 for offset, _ in refs)
    print(f"payload=0x{payload:x} cjk_strings={len(cjk)} aligned_refs={len(refs)}")
    if args.dump is not None:
        print("word dump:")
        for offset in range(args.dump, args.dump + args.dump_count * 4, 4):
            value = struct.unpack_from("<I", blob, offset)[0]
            resolved = resolve_string(blob, value, payload, args.restart_start)
            suffix = f" {resolved!r}" if resolved is not None else ""
            print(f"  file=0x{offset:06x} word=0x{value:08x}{suffix}")
    print("reference pages:")
    for page, count in pages.most_common(30):
        print(f"  0x{page << 12:06x}-0x{(page + 1) << 12:06x}: {count}")

    print("\nlikely pointer pairs:")
    shown = 0
    for ref_offset, zh_address in refs:
        zh = cjk[zh_address][1]
        candidates = []
        for delta in (-16, -12, -8, -4, 4, 8, 12, 16):
            neighbor_offset = ref_offset + delta
            if not (payload <= neighbor_offset <= len(blob) - 4):
                continue
            neighbor_address = struct.unpack_from("<I", blob, neighbor_offset)[0]
            neighbor = resolve_string(
                blob, neighbor_address, payload, args.restart_start
            )
            if neighbor and not CJK_RE.search(neighbor):
                candidates.append((delta, neighbor_address, neighbor))
        if not candidates:
            continue
        joined = " | ".join(
            f"{delta:+d}:0x{address:08x}={value!r}"
            for delta, address, value in candidates
        )
        print(
            f"ref=0x{ref_offset:06x} zh_addr=0x{zh_address:08x} "
            f"zh={zh!r} :: {joined}"
        )
        shown += 1
        if shown >= args.limit:
            break


if __name__ == "__main__":
    main()
