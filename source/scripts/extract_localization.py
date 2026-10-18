#!/usr/bin/env python3
"""Inspect UTF-8 strings and likely localization pairs in an ML autoexec.bin."""

from __future__ import annotations

import argparse
import re
import zipfile
from pathlib import Path


CJK_RE = re.compile(r"[\u3400-\u4dbf\u4e00-\u9fff]")
PRINTABLE_RE = re.compile(r"^[^\x00-\x08\x0b\x0c\x0e-\x1f\x7f]+$")


def strings(blob: bytes, minimum: int = 2):
    for match in re.finditer(rb"[^\x00]{%d,}\x00" % minimum, blob):
        raw = match.group()[:-1]
        try:
            value = raw.decode("utf-8")
        except UnicodeDecodeError:
            continue
        if PRINTABLE_RE.match(value):
            yield match.start(), value


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    parser.add_argument("--start", type=lambda v: int(v, 0), default=0)
    parser.add_argument("--end", type=lambda v: int(v, 0))
    parser.add_argument("--cjk-only", action="store_true")
    parser.add_argument("--ascii", action="store_true", help="escape non-ASCII output")
    parser.add_argument("--limit", type=int, default=200)
    args = parser.parse_args()

    with zipfile.ZipFile(args.archive) as archive:
        blob = archive.read("autoexec.bin")

    end = len(blob) if args.end is None else min(args.end, len(blob))
    count = 0
    for offset, value in strings(blob[args.start:end]):
        if args.cjk_only and not CJK_RE.search(value):
            continue
        shown = ascii(value) if args.ascii else repr(value)
        print(f"0x{args.start + offset:08x}\t{shown}")
        count += 1
        if count >= args.limit:
            break


if __name__ == "__main__":
    main()
