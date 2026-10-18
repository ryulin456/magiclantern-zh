#!/usr/bin/env python3
"""Generate an external module Summary/Description translation catalog."""

from __future__ import annotations

import argparse
import ast
import re
from pathlib import Path

from audit_module_text_layout import MODULE_ROW_RE
from module_translations_zh_cn import MODULE_TRANSLATIONS


STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"')
PAIR_RE = re.compile(
    r"static char __module_string_(?P<key>[a-z]+)_name.*?=\s*"
    r"(?P<name>(?:\"(?:\\.|[^\"\\])*\"\s*)+);\s*"
    r"static char __module_string_(?P=key)_value.*?=\s*"
    r"(?P<value>(?:\"(?:\\.|[^\"\\])*\"\s*)+);",
    re.S,
)


def decode(expression: str) -> str:
    return "".join(ast.literal_eval(token) for token in STRING_RE.findall(expression))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source_root", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    module_rows = {}
    header = args.source_root / "src" / "zh_cn_modules.h"
    for line in header.read_text(encoding="utf-8").splitlines():
        match = MODULE_ROW_RE.match(line)
        if match:
            name, summary, description = match.groups()
            module_rows[name] = (decode(f'"{summary}"'), decode(f'"{description}"'))

    catalogs = {}
    for path in args.source_root.glob("modules/**/build/module_strings.h"):
        module_name = path.parent.parent.name
        if module_name not in module_rows:
            continue
        pairs = {
            decode(match.group("name")): decode(match.group("value"))
            for match in PAIR_RE.finditer(path.read_text(encoding="utf-8"))
        }
        if "Summary" in pairs and "Description" in pairs:
            catalogs[module_name] = pairs

    missing = sorted(module_rows.keys() - catalogs.keys())
    if missing:
        raise SystemExit(f"module metadata headers not found: {missing}")

    rows = []
    existing_sources = {source for source, _ in MODULE_TRANSLATIONS}
    for name in sorted(module_rows):
        chinese_summary, chinese_description = module_rows[name]
        if catalogs[name]["Summary"] not in existing_sources:
            rows.append((catalogs[name]["Summary"], chinese_summary))
        rows.append((catalogs[name]["Description"], chinese_description))

    if len(rows) != 36 or len({source for source, _ in rows}) != len(rows):
        raise SystemExit("expected 36 new unique module metadata source strings")

    lines = [
        '"""Reviewed external translations for module summaries/descriptions."""',
        "",
        "MODULE_METADATA_TRANSLATIONS = (",
    ]
    lines.extend(f"    ({source!r}, {translated!r})," for source, translated in rows)
    lines.extend((")", ""))
    args.output.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"OK: wrote {len(rows)} module metadata translations to {args.output}")


if __name__ == "__main__":
    main()
