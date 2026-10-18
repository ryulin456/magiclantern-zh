#!/usr/bin/env python3
"""Package a localized legacy Magic Lantern build without changing its payload set."""
from __future__ import annotations
import argparse
import hashlib
import zipfile
from pathlib import Path

def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()

def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("original", type=Path); p.add_argument("build_root", type=Path)
    p.add_argument("output", type=Path); p.add_argument("sym_name"); p.add_argument("version")
    args = p.parse_args(); out = {}
    with zipfile.ZipFile(args.original) as z:
        original_names = set(z.namelist())
        for name in z.namelist():
            out[name] = z.read(name)
    def put(name: str, path: Path) -> None:
        if not path.is_file(): raise SystemExit(f"missing replacement: {path}")
        out[name] = path.read_bytes()
    autoexec_candidates = list(args.build_root.rglob("autoexec.bin"))
    if not autoexec_candidates: autoexec_candidates = list(args.build_root.rglob("magiclantern.bin"))
    if not autoexec_candidates: raise SystemExit("missing autoexec build")
    put("autoexec.bin", autoexec_candidates[0])
    sym_candidates = list(args.build_root.rglob(args.sym_name))
    if not sym_candidates: raise SystemExit(f"missing sym build: {args.sym_name}")
    put(f"ML/modules/{args.sym_name}", sym_candidates[0])
    module_names = sorted(name.removeprefix("ML/modules/").removesuffix(".mo") for name in original_names if name.startswith("ML/modules/") and name.endswith(".mo"))
    for name in module_names:
        candidates = list(args.build_root.rglob(f"{name}.mo"))
        if not candidates: raise SystemExit(f"missing module build: {name}")
        put(f"ML/modules/{name}.mo", candidates[0])
    assets = Path(__file__).resolve().parent.parent
    put("ML/fonts/zh_cn.lng", assets / "magiclantern-zh-src/data/fonts/zh_cn.lng")
    put("ML/fonts/zh_cn.bfn", assets / "magiclantern-zh-src/data/fonts/zh_cn.bfn")
    put("ML/doc/OFL-NotoSansSC.txt", assets / "magiclantern-zh-src/doc/OFL-NotoSansSC.txt")
    if len(out) != len(original_names) + 3: raise SystemExit("unexpected archive entry count change")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, "w", zipfile.ZIP_DEFLATED) as z:
        for name in sorted(out): z.writestr(name, out[name])
    print(f"wrote {args.output} ({len(out)} files, {sha256(args.output)}) version={args.version}")

if __name__ == "__main__": main()
