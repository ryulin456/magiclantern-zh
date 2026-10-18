#!/usr/bin/env python3
"""Download a large file in verified parallel HTTP ranges."""

from __future__ import annotations

import argparse
import concurrent.futures
import math
import time
import urllib.request
from pathlib import Path


def download_part(url: str, path: Path, start: int, end: int) -> Path:
    expected = end - start + 1
    if path.exists() and path.stat().st_size == expected:
        return path

    for attempt in range(4):
        try:
            request = urllib.request.Request(
                url,
                headers={
                    "Range": f"bytes={start}-{end}",
                    "User-Agent": "Mozilla/5.0 range-downloader",
                },
            )
            with urllib.request.urlopen(request, timeout=60) as response:
                if response.status != 206:
                    raise RuntimeError(f"server returned HTTP {response.status}, not 206")
                content_range = response.headers.get("Content-Range", "")
                if not content_range.startswith(f"bytes {start}-{end}/"):
                    raise RuntimeError(f"unexpected Content-Range: {content_range!r}")
                with path.open("wb") as output:
                    while True:
                        block = response.read(1024 * 1024)
                        if not block:
                            break
                        output.write(block)
            if path.stat().st_size != expected:
                raise RuntimeError(
                    f"short range {start}-{end}: {path.stat().st_size} != {expected}"
                )
            return path
        except Exception:
            if attempt == 3:
                raise
            time.sleep(2 ** attempt)
    raise AssertionError("unreachable")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("url")
    parser.add_argument("output", type=Path)
    parser.add_argument("size", type=int)
    parser.add_argument("--chunks", type=int, default=16)
    args = parser.parse_args()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    parts_dir = args.output.with_name(args.output.name + ".parts")
    parts_dir.mkdir(exist_ok=True)
    part_size = math.ceil(args.size / args.chunks)
    jobs = []
    for index in range(args.chunks):
        start = index * part_size
        end = min(args.size - 1, (index + 1) * part_size - 1)
        if start > end:
            break
        jobs.append(
            (
                args.url,
                parts_dir / f"part-{index:03d}.bin",
                start,
                end,
            )
        )

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.chunks) as pool:
        futures = [pool.submit(download_part, *job) for job in jobs]
        for completed, future in enumerate(concurrent.futures.as_completed(futures), 1):
            path = future.result()
            print(f"range {completed}/{len(futures)} complete: {path.name}", flush=True)

    with args.output.open("wb") as output:
        for _, path, _, _ in jobs:
            with path.open("rb") as part:
                while True:
                    block = part.read(1024 * 1024)
                    if not block:
                        break
                    output.write(block)

    actual = args.output.stat().st_size
    if actual != args.size:
        raise SystemExit(f"combined size mismatch: {actual} != {args.size}")

    for _, path, _, _ in jobs:
        path.unlink()
    print(f"downloaded {args.output} ({actual} bytes)")


if __name__ == "__main__":
    main()
