#!/usr/bin/env python3
"""Audit compiled core menu text against the shared zh-CN dictionary.

This deliberately scans only menu-facing source constructs, then confirms that
the English text is present in the target's original binary.  The binary filter
avoids requiring translations for code excluded by that camera's build flags.
Module menus are checked by audit_module_menu_translations.py instead.
"""

from __future__ import annotations

import argparse
import ast
import re
from pathlib import Path

from audit_translation_compatibility import fnv1a, target_strings
from validate_localization import load_translations


STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"')
ASSIGNMENT_RE = re.compile(
    r"\.(name|help|help2)\s*=\s*((?:\"(?:\\.|[^\"\\])*\"\s*)+)",
    re.S,
)
MENU_MACRO_RE = re.compile(
    r"\b(?:CHOICES|MENU_SET_VALUE|MENU_SET_RINFO|MENU_SET_WARNING)\s*\("
)
HAS_WORD_RE = re.compile(r"[A-Za-z]{2}")


def decode_c_strings(expression: str) -> str:
    """Decode and concatenate ordinary C string literals."""
    pieces = []
    for token in STRING_RE.findall(expression):
        try:
            value = ast.literal_eval(token)
        except (SyntaxError, ValueError):
            return ""
        if not isinstance(value, str):
            return ""
        pieces.append(value)
    return "".join(pieces)


def balanced_call(text: str, open_paren: int) -> str:
    depth = 0
    in_string = False
    escaped = False
    for index in range(open_paren, len(text)):
        char = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            continue
        if char == '"':
            in_string = True
        elif char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
            if depth == 0:
                return text[open_paren + 1 : index]
    return ""


def source_candidates(path: Path) -> list[tuple[int, str, str]]:
    text = path.read_text(encoding="utf-8", errors="strict")
    rows: list[tuple[int, str, str]] = []
    for match in ASSIGNMENT_RE.finditer(text):
        value = decode_c_strings(match.group(2))
        if value:
            rows.append((text.count("\n", 0, match.start()) + 1, match.group(1), value))

    for match in MENU_MACRO_RE.finditer(text):
        open_paren = text.find("(", match.start(), match.end() + 1)
        call = balanced_call(text, open_paren)
        for token in STRING_RE.findall(call):
            value = decode_c_strings(token)
            if value:
                rows.append(
                    (
                        text.count("\n", 0, match.start()) + 1,
                        match.group().split("(", 1)[0].strip(),
                        value,
                    )
                )
    return rows


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("language_file", type=Path)
    parser.add_argument("source_root", type=Path)
    parser.add_argument("model")
    parser.add_argument("target_zip", type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()

    roots = [args.source_root / "src", args.source_root / "platform" / args.model]
    candidates: dict[str, list[tuple[Path, int, str]]] = {}
    for root in roots:
        for path in sorted(root.rglob("*")):
            if path.suffix.lower() not in {".c", ".h"}:
                continue
            if "build" in path.parts:
                continue
            for line, kind, value in source_candidates(path):
                if HAS_WORD_RE.search(value):
                    candidates.setdefault(value, []).append((path, line, kind))

    translations = load_translations(args.language_file)
    translated_hashes = {hash_value for hash_value, _ in translations}
    candidate_hashes = {fnv1a(value) for value in candidates}
    present = target_strings(args.target_zip, candidate_hashes)

    compiled = {
        value: refs
        for value, refs in candidates.items()
        if fnv1a(value) in present
        and value in present[fnv1a(value)]
    }
    missing = {
        value: refs
        for value, refs in compiled.items()
        if fnv1a(value) not in translated_hashes
    }

    lines = [
        f"model\t{args.model}",
        f"source_candidates\t{len(candidates)}",
        f"compiled_menu_strings\t{len(compiled)}",
        f"translated\t{len(compiled) - len(missing)}",
        f"missing\t{len(missing)}",
    ]
    for value, refs in sorted(missing.items()):
        locations = ", ".join(
            f"{path.relative_to(args.source_root).as_posix()}:{line}:{kind}"
            for path, line, kind in refs
        )
        lines.append(f"MISSING\t{value!r}\t{locations}")

    output = "\n".join(lines) + "\n"
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(output, encoding="utf-8", newline="\n")
    print(output, end="")
    if missing:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
