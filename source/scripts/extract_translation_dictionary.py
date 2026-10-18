#!/usr/bin/env python3
"""Recover a hash-based Chinese translation dictionary from ML binaries."""

from __future__ import annotations

import argparse
import binascii
import re
import struct
import zipfile
from pathlib import Path

from analyze_ml_binaries import find_payload, read_symbols
from find_translation_table import CJK_RE, PRINTABLE_RE, resolve_string


def fnv1a(data: bytes) -> int:
    value = 0x811C9DC5
    for byte in data:
        value ^= byte
        value = (value * 0x01000193) & 0xFFFFFFFF
    return value


def fnv1(data: bytes) -> int:
    value = 0x811C9DC5
    for byte in data:
        value = (value * 0x01000193) & 0xFFFFFFFF
        value ^= byte
    return value


def djb2(data: bytes) -> int:
    value = 5381
    for byte in data:
        value = ((value << 5) + value + byte) & 0xFFFFFFFF
    return value


def sdbm(data: bytes) -> int:
    value = 0
    for byte in data:
        value = (byte + (value << 6) + (value << 16) - value) & 0xFFFFFFFF
    return value


def jenkins(data: bytes) -> int:
    value = 0
    for byte in data:
        value = (value + byte) & 0xFFFFFFFF
        value = (value + (value << 10)) & 0xFFFFFFFF
        value ^= value >> 6
    value = (value + (value << 3)) & 0xFFFFFFFF
    value ^= value >> 11
    value = (value + (value << 15)) & 0xFFFFFFFF
    return value


HASHES = {
    "crc32": lambda data: binascii.crc32(data) & 0xFFFFFFFF,
    "fnv1a": fnv1a,
    "fnv1": fnv1,
    "djb2": djb2,
    "sdbm": sdbm,
    "jenkins": jenkins,
}


def ascii_strings(blob: bytes, minimum: int = 1):
    pattern = rb"[\t\r\n\x20-\x7e]{%d,}\x00" % minimum
    for match in re.finditer(pattern, blob):
        value = match.group()[:-1].decode("ascii")
        if value:
            yield match.start(), value


def ascii_suffixes(blob: bytes):
    """Yield every printable suffix, accounting for linker string merging."""
    for offset, value in ascii_strings(blob):
        for index in range(len(value)):
            yield offset + index, value[index:]


def candidate_strings(path: Path):
    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as archive:
            for info in archive.infolist():
                if info.is_dir():
                    continue
                blob = archive.read(info)
                for offset, value in ascii_suffixes(blob):
                    yield info.filename, offset, value
    else:
        for offset, value in ascii_suffixes(path.read_bytes()):
            yield path.name, offset, value


def find_table(
    blob: bytes,
    payload: int,
    restart_start: int,
) -> list[tuple[int, int, str]]:
    best: list[tuple[int, int, str]] = []
    for layout in ("hash-pointer", "pointer-hash"):
        for start in range(payload, len(blob) - 7, 4):
            rows: list[tuple[int, int, str]] = []
            previous_hash = -1
            for offset in range(start, len(blob) - 7, 8):
                first, second = struct.unpack_from("<II", blob, offset)
                if layout == "hash-pointer":
                    hash_value, address = first, second
                else:
                    address, hash_value = first, second
                value = resolve_string(blob, address, payload, restart_start)
                if value is None or hash_value <= previous_hash:
                    break
                rows.append((address, hash_value, value))
                previous_hash = hash_value
            if len(rows) > len(best):
                best = rows
    return best


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("localized_autoexec", type=Path)
    parser.add_argument("localized_sym", type=Path)
    parser.add_argument("original_files", type=Path)
    parser.add_argument("restart_start", type=lambda value: int(value, 0))
    parser.add_argument("--limit", type=int, default=100)
    args = parser.parse_args()

    localized = args.localized_autoexec.read_bytes()
    symbols = read_symbols(args.localized_sym)
    payload = find_payload(localized, symbols, args.restart_start)
    rows = find_table(localized, payload, args.restart_start)
    hashes = {hash_value for _, hash_value, _ in rows}
    original_strings = list(candidate_strings(args.original_files))

    print(
        f"payload=0x{payload:x} dictionary_entries={len(rows)} "
        f"hash_range=0x{rows[0][1]:08x}..0x{rows[-1][1]:08x}"
    )
    scored = []
    for name, function in HASHES.items():
        index: dict[int, list[tuple[str, int, str]]] = {}
        for source, offset, value in original_strings:
            index.setdefault(function(value.encode("ascii")), []).append(
                (source, offset, value)
            )
        score = len(hashes.intersection(index))
        scored.append((score, name, index))
        print(f"{name}: matched {score}/{len(rows)} dictionary hashes")

    score, name, index = max(scored)
    print(f"\nbest={name} matches={score}")
    shown = 0
    for _, hash_value, translated in rows:
        matches = index.get(hash_value)
        if not matches:
            continue
        original_values = sorted({value for _, _, value in matches})
        print(
            f"0x{hash_value:08x}\t{original_values!r}\t{translated!r}"
        )
        shown += 1
        if shown >= args.limit:
            break


if __name__ == "__main__":
    main()
