#!/usr/bin/env python3
"""Check that every packaged ML module dependency has a symbol provider."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


def read_main_symbols(path: Path) -> set[str]:
    symbols: set[str] = set()
    for line in path.read_text(encoding="ascii", errors="strict").splitlines():
        fields = line.split(maxsplit=1)
        if len(fields) == 2:
            symbols.add(fields[1])
    return symbols


def read_module_symbols(
    path: Path, readelf: Path
) -> tuple[set[str], set[str]]:
    result = subprocess.run(
        [str(readelf), "-sW", str(path)],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="strict",
    )
    defined: set[str] = set()
    undefined: set[str] = set()
    for line in result.stdout.splitlines():
        fields = line.split(maxsplit=7)
        if len(fields) != 8 or not fields[0].endswith(":"):
            continue
        binding = fields[4]
        section = fields[6]
        name = fields[7]
        if binding not in {"GLOBAL", "WEAK"} or not name:
            continue
        if section == "UND":
            undefined.add(name)
        else:
            defined.add(name)
    if not defined and not undefined:
        raise SystemExit(f"module has no readable symbols: {path}")
    return defined, undefined


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sym", type=Path)
    parser.add_argument("modules_dir", type=Path)
    parser.add_argument("readelf", type=Path)
    parser.add_argument("--include-hidden", action="store_true")
    parser.add_argument("--expected-count", type=int)
    parser.add_argument("--included-policy", type=Path)
    parser.add_argument("--hidden-policy", type=Path)
    parser.add_argument(
        "--exclude-hidden-marked",
        action="store_true",
        help=(
            "exclude modules with zero-byte .hid policy markers from the "
            "loadability/symbol audit; their .mo files remain checked against "
            "the included policy"
        ),
    )
    parser.add_argument(
        "--audit-policy",
        type=Path,
        action="append",
        default=[],
        help="audit only the union of module names in these policy files",
    )
    parser.add_argument("--require-arkanoid-guard", action="store_true")
    args = parser.parse_args()

    modules = sorted(args.modules_dir.glob("*.mo"))
    if args.include_hidden:
        # Some packages use non-empty .hid files for disabled module binaries;
        # zero-byte .hid files are policy markers and are not ELF modules.
        modules += sorted(
            path for path in args.modules_dir.glob("*.hid") if path.stat().st_size
        )

    def policy_names(path: Path) -> set[str]:
        return {
            line.strip()
            for line in path.read_text(encoding="utf-8").splitlines()
            if line.strip() and not line.lstrip().startswith("#")
        }

    packaged_modules = list(modules)

    if args.audit_policy:
        audit_names: set[str] = set()
        for path in args.audit_policy:
            audit_names.update(policy_names(path))
        by_name = {path.stem: path for path in modules}
        missing = audit_names - by_name.keys()
        if missing:
            raise SystemExit(f"audit policy modules not built: {sorted(missing)}")
        modules = [by_name[name] for name in sorted(audit_names)]

    if args.included_policy:
        expected = policy_names(args.included_policy)
        actual = {path.stem for path in args.modules_dir.glob("*.mo")}
        if actual != expected:
            raise SystemExit(
                f"active module policy mismatch: missing={sorted(expected-actual)}, "
                f"extra={sorted(actual-expected)}"
            )
    if args.hidden_policy:
        expected = policy_names(args.hidden_policy)
        actual = {path.stem for path in args.modules_dir.glob("*.hid")}
        if actual != expected:
            raise SystemExit(
                f"hidden module policy mismatch: missing={sorted(expected-actual)}, "
                f"extra={sorted(actual-expected)}"
            )

    hidden_markers: set[str] = set()
    if args.exclude_hidden_marked:
        hidden_markers = {
            path.stem
            for path in args.modules_dir.glob("*.hid")
            if path.stat().st_size == 0
        }
        # A zero-byte .hid file is a GUI visibility marker, not a module
        # binary.  If a same-name .mo is also present (for example dual_iso
        # on 750D), that .mo still loads and must remain in the symbol audit.
        modules = [
            path
            for path in modules
            if not (path.suffix == ".hid" and path.stem in hidden_markers)
        ]

    expected_count = args.expected_count
    if (
        expected_count is None
        and not args.included_policy
        and not args.hidden_policy
        and not args.audit_policy
    ):
        expected_count = 19
    if expected_count is not None and len(modules) != expected_count:
        raise SystemExit(
            f"expected {expected_count} packaged module artifacts, found {len(modules)}"
        )

    module_symbols = {
        path: read_module_symbols(path, args.readelf) for path in modules
    }
    providers = read_main_symbols(args.sym)
    for defined, _ in module_symbols.values():
        providers.update(defined)

    missing_by_module: dict[Path, list[str]] = {}
    for path, (_, undefined) in module_symbols.items():
        missing = sorted(undefined - providers)
        if missing:
            missing_by_module[path] = missing

    if missing_by_module:
        lines = ["unresolved module symbols:"]
        for path, missing in missing_by_module.items():
            lines.append(f"  {path.name}: {', '.join(missing)}")
        raise SystemExit("\n".join(lines))

    if args.require_arkanoid_guard:
        arkanoid_paths = [path for path in modules if path.stem == "arkanoid"]
        if not arkanoid_paths:
            raise SystemExit("packaged Arkanoid module not found")
        arkanoid_undefined = module_symbols[arkanoid_paths[0]][1]
        required_guard_symbols = {"NotifyBox", "lv"}
        if not required_guard_symbols <= arkanoid_undefined:
            raise SystemExit(
                "Arkanoid LiveView guard dependencies missing: "
                + ", ".join(sorted(required_guard_symbols - arkanoid_undefined))
            )

    print(
        f"OK: {len(modules)} loadable modules"
        + (
            f" ({len(packaged_modules)} packaged module binaries; "
            f"{len(hidden_markers)} zero-byte GUI visibility markers)"
            if args.exclude_hidden_marked
            else ""
        )
        + "; all undefined symbols have providers"
        + ("; Arkanoid LiveView guard is linked" if args.require_arkanoid_guard else "")
    )


if __name__ == "__main__":
    main()
