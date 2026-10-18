#!/usr/bin/env python3
"""Audit and copy the 2025-06-20 multi-camera zh-CN TEST-ONLY builds."""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import zipfile
from pathlib import Path, PurePosixPath


MODELS = (
    "7D2.112",
    "50D.109",
    "60D.111",
    "70D.112",
    "100D.101",
    "200D.101",
    "500D.111",
    "550D.109",
    "600D.102",
    "650D.104",
    "700D.115",
    "750D.110",
    "1100D.105",
    "EOSM.202",
)

QUARANTINED: dict[str, set[str]] = {}

RESOURCE_FILES = {
    "ML/fonts/zh_cn.lng": Path("magiclantern-zh-src/data/fonts/zh_cn.lng"),
    "ML/fonts/zh_cn.bfn": Path("magiclantern-zh-src/data/fonts/zh_cn.bfn"),
    "ML/doc/OFL-NotoSansSC.txt": Path("magiclantern-zh-src/doc/OFL-NotoSansSC.txt"),
}


def policy_names(path: Path) -> set[str]:
    return {
        line.strip()
        for line in path.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    }


def file_members(archive: zipfile.ZipFile) -> dict[str, bytes]:
    names = [item.filename for item in archive.infolist()]
    if len(names) != len(set(names)):
        raise SystemExit(f"duplicate ZIP member names: {archive.filename}")
    for name in names:
        path = PurePosixPath(name)
        if path.is_absolute() or ".." in path.parts or "\\" in name:
            raise SystemExit(f"unsafe ZIP member {name!r}: {archive.filename}")
    broken = archive.testzip()
    if broken:
        raise SystemExit(f"ZIP CRC failure in {broken}: {archive.filename}")
    return {
        item.filename: archive.read(item)
        for item in archive.infolist()
        if not item.is_dir()
    }


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source_root", type=Path)
    parser.add_argument("output_dir", type=Path)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--models", nargs="+", choices=MODELS, default=list(MODELS))
    parser.add_argument("--revision", default="r1")
    args = parser.parse_args()

    source_root = args.source_root.resolve()
    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    records: list[dict[str, object]] = []

    for model in args.models:
        version = f"2025-06-20.{model}"
        original_path = source_root / f"magiclantern.{version}.zip"
        build_root = source_root / "magiclantern-zh-src" / "platform" / model
        localized_path = build_root / "build" / "magiclantern.zip"

        with zipfile.ZipFile(original_path) as original_zip:
            original = file_members(original_zip)
        with zipfile.ZipFile(localized_path) as localized_zip:
            localized = file_members(localized_zip)

        included = policy_names(build_root / "modules.included")
        hidden = policy_names(build_root / "modules.hidden")
        actual_modules = {
            PurePosixPath(name).stem
            for name in localized
            if name.startswith("ML/modules/") and name.endswith(".mo")
        }
        actual_hidden = {
            PurePosixPath(name).stem
            for name in localized
            if name.startswith("ML/modules/") and name.endswith(".hid")
        }
        if actual_modules != included:
            raise SystemExit(
                f"{model}: module policy mismatch: "
                f"missing={sorted(included-actual_modules)}, "
                f"extra={sorted(actual_modules-included)}"
            )
        if actual_hidden != hidden:
            raise SystemExit(
                f"{model}: hidden policy mismatch: "
                f"missing={sorted(hidden-actual_hidden)}, "
                f"extra={sorted(actual_hidden-hidden)}"
            )
        nonzero_hidden = [
            name
            for name, data in localized.items()
            if name.startswith("ML/modules/")
            and name.endswith(".hid")
            and data
        ]
        if nonzero_hidden:
            raise SystemExit(f"{model}: nonzero hidden markers: {nonzero_hidden}")

        for member, source in RESOURCE_FILES.items():
            expected = source_root / source
            if localized.get(member) != expected.read_bytes():
                raise SystemExit(f"{model}: missing or stale resource {member}")

        autoexec = localized.get("autoexec.bin", b"")
        if version.encode("ascii") not in autoexec:
            raise SystemExit(f"{model}: embedded version is not {version}")

        expected_removed = {
            f"ML/modules/{name}.mo" for name in QUARANTINED.get(model, set())
        }
        expected_added = set(RESOURCE_FILES)
        expected_added.update(
            f"ML/modules/{name}.hid"
            for name in QUARANTINED.get(model, set())
            if f"ML/modules/{name}.hid" not in original
        )
        removed = set(original) - set(localized)
        added = set(localized) - set(original)
        if removed != expected_removed:
            raise SystemExit(
                f"{model}: unexpected removed files: "
                f"expected={sorted(expected_removed)}, actual={sorted(removed)}"
            )
        if added != expected_added:
            raise SystemExit(
                f"{model}: unexpected added files: "
                f"expected={sorted(expected_added)}, actual={sorted(added)}"
            )

        changed = {
            name
            for name in set(original) & set(localized)
            if original[name] != localized[name]
        }
        allowed_changed = {"autoexec.bin"}
        allowed_changed.update(
            name
            for name in localized
            if name.startswith("ML/modules/")
            and (name.endswith(".mo") or name.endswith(".sym"))
        )
        unexpected_changed = changed - allowed_changed
        if unexpected_changed:
            raise SystemExit(
                f"{model}: unrelated files changed: {sorted(unexpected_changed)}"
            )
        if "ML-SETUP.FIR" in original and (
            localized.get("ML-SETUP.FIR") != original["ML-SETUP.FIR"]
        ):
            raise SystemExit(f"{model}: ML-SETUP.FIR changed")

        output_name = (
            f"magiclantern.{version}.zh-CN.{args.revision}-TEST-ONLY.zip"
        )
        output_path = output_dir / output_name
        shutil.copyfile(localized_path, output_path)
        output_hash = sha256(output_path)
        hash_path = output_path.with_suffix(".sha256.txt")
        hash_path.write_text(
            f"{output_hash}  {output_name}\n", encoding="ascii", newline="\n"
        )

        records.append(
            {
                "model": model,
                "version": version,
                "output": output_name,
                "sha256": output_hash,
                "size": output_path.stat().st_size,
                "modules": len(actual_modules),
                "hidden_markers": len(actual_hidden),
                "quarantined": sorted(QUARANTINED.get(model, set())),
                "added": sorted(added),
                "removed": sorted(removed),
                "changed": sorted(changed),
            }
        )
        print(
            f"OK {model}: {len(actual_modules)} modules, "
            f"{len(actual_hidden)} hidden markers, SHA-256 {output_hash}"
        )

    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(
        json.dumps(records, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )


if __name__ == "__main__":
    main()
