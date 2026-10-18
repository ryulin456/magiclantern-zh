#!/usr/bin/env python3
"""Audit whether a shared ML translation dictionary is safe for a target ZIP."""

from __future__ import annotations

import argparse
import re
import zipfile
from pathlib import Path

from validate_localization import load_translations


ASCII_STRING_RE = re.compile(rb"[\t\r\n\x20-\x7e]{2,}\0")
PRINTF_RE = re.compile(
    r"%(?:\d+\$)?[-+ #0']*(?:\*|\d+)?(?:\.(?:\*|\d+))?"
    r"(?:hh|h|ll|l|j|z|t|L)?[diuoxXfFeEgGaAcspn%]"
)


def fnv1a(value: str) -> int:
    result = 0x811C9DC5
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 0x01000193) & 0xFFFFFFFF
    return result


def format_tokens(value: str) -> tuple[str, ...]:
    # A percentage written after a number is user text, not a printf token.
    # Without this normalization, phrases such as "5% of" are misread as
    # the valid C conversion "% o" (space flag + octal conversion).
    normalized = re.sub(r"(?<=\d)%(?=\s|$|[,.!?;:])", "%%", value)
    return tuple(token for token in PRINTF_RE.findall(normalized) if token != "%%")


def target_strings(path: Path, wanted_hashes: set[int]) -> dict[int, set[str]]:
    by_hash: dict[int, set[str]] = {}

    def consider(value: str) -> None:
        hash_value = fnv1a(value)
        if hash_value in wanted_hashes:
            by_hash.setdefault(hash_value, set()).add(value)

    with zipfile.ZipFile(path) as archive:
        for info in archive.infolist():
            if info.is_dir() or not (
                info.filename == "autoexec.bin"
                or info.filename.lower().endswith((".mo", ".hid"))
            ):
                continue
            blob = archive.read(info)
            for match in ASCII_STRING_RE.finditer(blob):
                value = match.group()[:-1].decode("ascii")
                consider(value)
                # Account for linker string merging and pointers to suffixes.
                # Cap pathological binary strings; real UI strings are short.
                suffix_limit = min(len(value), 512) if len(value) <= 1024 else 1
                for index in range(1, suffix_limit):
                    suffix = value[index:]
                    if len(suffix) >= 2 and suffix[0].isprintable():
                        consider(suffix)
    return by_hash


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("language_file", type=Path)
    parser.add_argument("target_zip", type=Path)
    parser.add_argument("--minimum-matches", type=int, default=0)
    args = parser.parse_args()

    translations = load_translations(args.language_file)
    by_hash = target_strings(args.target_zip, {row[0] for row in translations})

    matched = []
    mismatches = []
    for hash_value, translated in translations:
        originals = by_hash.get(hash_value)
        if not originals:
            continue
        matched.append(hash_value)
        translated_tokens = format_tokens(translated)
        if not any(format_tokens(original) == translated_tokens for original in originals):
            mismatches.append((hash_value, originals, translated, translated_tokens))

    if len(matched) < args.minimum_matches:
        raise SystemExit(
            f"translation coverage too low: {len(matched)} matches, "
            f"minimum is {args.minimum_matches}"
        )
    if mismatches:
        lines = ["format placeholder mismatches:"]
        for hash_value, originals, translated, tokens in mismatches:
            lines.append(
                f"  0x{hash_value:08x}: {sorted(originals)!r} -> "
                f"{translated!r} {tokens!r}"
            )
        raise SystemExit("\n".join(lines))

    print(
        f"OK: {len(matched)}/{len(translations)} dictionary entries occur in "
        f"the target package; all matched printf placeholders are compatible"
    )


if __name__ == "__main__":
    main()
