#!/usr/bin/env python3
"""Audit fixed module menu text against the external zh-CN dictionary.

This deliberately audits source-level menu fields instead of treating every
ASCII string in an ELF module as user interface text.  Dynamic MENU_SET_*
format strings are reported separately because an exact post-format hash lookup
cannot prove that every rendered value will be translated.
"""

from __future__ import annotations

import argparse
import ast
import re
from collections import defaultdict
from pathlib import Path

from validate_localization import load_translations


MODULE_DIRS = {
    "adv_int": "adv_int",
    "arkanoid": "arkanoid",
    "autoexpo": "autoexpo",
    "bench": "bench",
    "crop_rec": "crop_rec",
    "deflick": "deflick",
    "dot_tune": "dot_tune",
    "dual_iso": "dual_iso",
    "edmac": "dev_tools/edmac",
    "ettr": "ettr",
    "file_man": "file_man",
    "img_name": "img_name",
    "lua": "lua",
    "mlv_lite": "raw_video/mlv_lite",
    "mlv_play": "raw_video/mlv_play",
    "mlv_rec": "raw_video/mlv_rec",
    "mlv_snd": "raw_video/mlv_snd",
    "pic_view": "pic_view",
    "selftest": "selftest",
    "silent": "silent",
    "vid_af": "vid_af",
}

STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"', re.DOTALL)
CJK_RE = re.compile(r"[\u3400-\u4dbf\u4e00-\u9fff]")
FIELD_RE = re.compile(r"\.(name|help|choices)\s*=")
CALL_RE = re.compile(r"\b(MENU_SET_(?:NAME|VALUE|RINFO|HELP|WARNING)|CHOICES)\s*\(")
PRINTF_RE = re.compile(
    r"%(?:\d+\$)?[-+ #0']*(?:\*|\d+)?(?:\.(?:\*|\d+))?"
    r"(?:hh|h|ll|l|j|z|t|L)?[diuoxXfFeEgGaAcspn%]"
)
MODULE_STRING_PAIR_RE = re.compile(
    r"static char __module_string_(?P<key>[a-z]+)_name.*?=\s*"
    r"(?P<name>(?:\"(?:\\.|[^\"\\])*\"\s*)+);\s*"
    r"static char __module_string_(?P=key)_value.*?=\s*"
    r"(?P<value>(?:\"(?:\\.|[^\"\\])*\"\s*)+);",
    re.S,
)


def fnv1a(value: str) -> int:
    result = 0x811C9DC5
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 0x01000193) & 0xFFFFFFFF
    return result


def format_tokens(value: str) -> tuple[str, ...]:
    return tuple(token for token in PRINTF_RE.findall(value) if token != "%%")


def is_translated(value: str, translated_hashes: set[int]) -> bool:
    """Accept dictionary translations and reviewed Chinese source literals."""
    return bool(CJK_RE.search(value)) or fnv1a(value) in translated_hashes


def strip_comments(text: str) -> str:
    """Replace C comments with spaces while preserving strings and offsets."""
    out = list(text)
    index = 0
    state = "code"
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 1
                state = "line_comment"
            elif char == "/" and nxt == "*":
                out[index] = out[index + 1] = " "
                index += 1
                state = "block_comment"
        elif state == "string":
            if char == "\\":
                index += 1
            elif char == '"':
                state = "code"
        elif state == "char":
            if char == "\\":
                index += 1
            elif char == "'":
                state = "code"
        elif state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                out[index] = " "
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 1
                state = "code"
            elif char != "\n":
                out[index] = " "
        index += 1
    return "".join(out)


def scan_expression(text: str, start: int, closing: str | None) -> tuple[str, int]:
    """Return an expression ending at a top-level comma or matching delimiter."""
    pairs = {"(": ")", "[": "]", "{": "}"}
    stack: list[str] = []
    index = start
    in_string = False
    in_char = False
    while index < len(text):
        char = text[index]
        if in_string:
            if char == "\\":
                index += 1
            elif char == '"':
                in_string = False
        elif in_char:
            if char == "\\":
                index += 1
            elif char == "'":
                in_char = False
        elif char == '"':
            in_string = True
        elif char == "'":
            in_char = True
        elif char in pairs:
            stack.append(pairs[char])
        elif stack and char == stack[-1]:
            stack.pop()
        elif not stack and closing and char == closing:
            return text[start:index], index
        elif not stack and not closing and char == ",":
            return text[start:index], index
        index += 1
    return text[start:index], index


def split_arguments(text: str) -> list[str]:
    args: list[str] = []
    pairs = {"(": ")", "[": "]", "{": "}"}
    stack: list[str] = []
    start = 0
    index = 0
    in_string = False
    in_char = False
    while index < len(text):
        char = text[index]
        if in_string:
            if char == "\\":
                index += 1
            elif char == '"':
                in_string = False
        elif in_char:
            if char == "\\":
                index += 1
            elif char == "'":
                in_char = False
        elif char == '"':
            in_string = True
        elif char == "'":
            in_char = True
        elif char in pairs:
            stack.append(pairs[char])
        elif stack and char == stack[-1]:
            stack.pop()
        elif not stack and char == ",":
            args.append(text[start:index])
            start = index + 1
        index += 1
    args.append(text[start:])
    return args


def decode_literals(expression: str, concatenate: bool) -> list[str]:
    values: list[str] = []
    for match in STRING_RE.finditer(expression):
        try:
            values.append(ast.literal_eval(match.group()))
        except (SyntaxError, ValueError):
            continue
    if concatenate and values:
        return ["".join(values)]
    return values


def line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def candidates(path: Path) -> list[tuple[str, str, int, bool]]:
    original = path.read_text(encoding="utf-8", errors="replace")
    text = strip_comments(original)
    found: list[tuple[str, str, int, bool]] = []

    for match in FIELD_RE.finditer(text):
        kind = match.group(1)
        expression, _ = scan_expression(text, match.end(), None)
        if kind == "choices" and re.search(r"\bCHOICES\s*\(", expression):
            continue
        values = decode_literals(expression, concatenate=kind in {"name", "help"})
        for value in values:
            found.append((kind, value, line_number(text, match.start()), False))

    for match in CALL_RE.finditer(text):
        kind = match.group(1)
        expression, _ = scan_expression(text, match.end(), ")")
        args = split_arguments(expression)
        if kind == "CHOICES":
            values = []
            for arg in args:
                values.extend(decode_literals(arg, concatenate=True))
            dynamic = False
        else:
            format_index = 1 if kind == "MENU_SET_WARNING" else 0
            values = (
                decode_literals(args[format_index], concatenate=True)
                if len(args) > format_index
                else []
            )
            dynamic = len(args) > format_index + 1
        for value in values:
            found.append((kind, value, line_number(text, match.start()), dynamic))

    return found


def policy_names(paths: list[Path]) -> list[str]:
    names: set[str] = set()
    for path in paths:
        for raw in path.read_text(encoding="utf-8").splitlines():
            value = raw.split("#", 1)[0].strip()
            if value:
                names.add(value)
    unknown = names - MODULE_DIRS.keys()
    if unknown:
        raise SystemExit(f"unknown policy modules: {sorted(unknown)}")
    return sorted(names)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("language_file", type=Path)
    parser.add_argument("modules_root", type=Path)
    parser.add_argument("policies", type=Path, nargs="+")
    parser.add_argument("--show-covered", action="store_true")
    parser.add_argument("--show-dynamic", action="store_true")
    parser.add_argument("--tsv", action="store_true")
    parser.add_argument("--dynamic-only", action="store_true")
    args = parser.parse_args()

    translations = dict(load_translations(args.language_file))
    translated_hashes = set(translations)
    names = policy_names(args.policies)
    metadata_table = args.modules_root.parent / "src" / "zh_cn_modules.h"
    directly_translated_modules: set[str] = set()
    if metadata_table.is_file():
        directly_translated_modules = set(
            re.findall(
                r'^\s*\{\s*"([^"]+)"\s*,',
                metadata_table.read_text(encoding="utf-8"),
                re.M,
            )
        )
    fixed: dict[str, list[tuple[str, str, Path, int]]] = defaultdict(list)
    dynamic: dict[str, list[tuple[str, str, Path, int]]] = defaultdict(list)

    for name in names:
        root = args.modules_root / MODULE_DIRS[name]
        # Historical branches kept modules directly below modules/.
        legacy_root = args.modules_root / name
        if not root.is_dir() and legacy_root.is_dir():
            root = legacy_root
        readme = root / "README.rst"
        if readme.is_file():
            for line, value in enumerate(readme.read_text(encoding="utf-8").splitlines(), 1):
                if value.strip():
                    fixed[name].append(("module_name", value.strip(), readme, line))
                    break
        metadata = root / "module_strings.h"
        if not metadata.is_file():
            metadata = root / "build" / "module_strings.h"
        if metadata.is_file():
            metadata_text = metadata.read_text(encoding="utf-8")
            for match in MODULE_STRING_PAIR_RE.finditer(metadata_text):
                field = "".join(ast.literal_eval(token) for token in STRING_RE.findall(match.group("name")))
                if field not in {"Summary", "Description"} and not field.startswith("Help page"):
                    continue
                value = "".join(ast.literal_eval(token) for token in STRING_RE.findall(match.group("value")))
                fixed[name].append((f"metadata_{field.lower()}", value, metadata, 1))
        for path in sorted(root.rglob("*")):
            if path.suffix not in {".c", ".h"} or "build" in path.parts:
                continue
            relative_parts = path.relative_to(root).parts
            if name == "lua" and relative_parts and relative_parts[0] in {"lua", "dietlibc"}:
                continue
            for kind, value, line, is_dynamic in candidates(path):
                if not value or not (re.search(r"[A-Za-z]", value) or CJK_RE.search(value)):
                    continue
                row = (kind, value, path, line)
                (dynamic if is_dynamic else fixed)[name].append(row)

    unique_fixed: dict[str, dict[str, tuple[str, Path, int]]] = {}
    for name, rows in fixed.items():
        unique_fixed[name] = {
            value: (kind, path, line) for kind, value, path, line in rows
        }

    total = sum(len(rows) for rows in unique_fixed.values())
    covered = sum(
        is_translated(value, translated_hashes)
        for rows in unique_fixed.values()
        for value in rows
    )
    if args.tsv:
        emitted: set[tuple[str, str]] = set()
        for name in names:
            if not args.dynamic_only:
                for value, (kind, _, _) in sorted(unique_fixed.get(name, {}).items()):
                    if not is_translated(value, translated_hashes) and (kind, value) not in emitted:
                        print(f"{name}\t{kind}\t{value}")
                        emitted.add((kind, value))
            if args.show_dynamic or args.dynamic_only:
                for kind, value, _, _ in dynamic.get(name, []):
                    if not is_translated(value, translated_hashes) and (kind, value) not in emitted:
                        print(f"{name}\t{kind}\t{value}")
                        emitted.add((kind, value))
        return
    unique_dynamic = {
        (kind, value)
        for rows in dynamic.values()
        for kind, value, _, _ in rows
    }
    covered_dynamic = sum(
        is_translated(value, translated_hashes) for _, value in unique_dynamic
    )
    format_rows = {
        (kind, value)
        for rows in unique_fixed.values()
        for value, (kind, _, _) in rows.items()
        if kind.startswith("MENU_SET_")
    } | unique_dynamic
    format_errors = []
    for kind, value in sorted(format_rows):
        translated = translations.get(fnv1a(value))
        if translated is None:
            continue
        if format_tokens(value) != format_tokens(translated):
            format_errors.append((kind, value, translated))
    if format_errors:
        lines = ["module translation placeholder mismatches:"]
        for kind, source, translated in format_errors:
            lines.append(f"  {kind}: {source!r} -> {translated!r}")
        raise SystemExit("\n".join(lines))
    print(f"fixed UI strings: {covered}/{total} translated")
    print(
        f"dynamic format strings: {covered_dynamic}/{len(unique_dynamic)} "
        f"translated ({sum(len(rows) for rows in dynamic.values())} call sites)"
    )
    print(f"format placeholders: {len(format_rows)}/{len(format_rows)} compatible")
    kinds: dict[str, list[bool]] = defaultdict(list)
    for rows in unique_fixed.values():
        for value, (kind, _, _) in rows.items():
            kinds[kind].append(is_translated(value, translated_hashes))
    for kind in sorted(kinds):
        print(f"  {kind}: {sum(kinds[kind])}/{len(kinds[kind])} translated")

    for name in names:
        rows = unique_fixed.get(name, {})
        missing = [value for value in rows if not is_translated(value, translated_hashes)]
        present = len(rows) - len(missing)
        print(f"{name}: {present}/{len(rows)} fixed strings translated")
        selected = sorted(rows) if args.show_covered else sorted(missing)
        for value in selected:
            if not args.show_covered and is_translated(value, translated_hashes):
                continue
            kind, path, line = rows[value]
            status = "OK" if is_translated(value, translated_hashes) else "MISSING"
            rel = path.relative_to(args.modules_root)
            print(f"  {status} {kind} {rel}:{line}: {value!r}")
        if args.show_dynamic:
            for kind, value, path, line in dynamic.get(name, []):
                if is_translated(value, translated_hashes):
                    continue
                rel = path.relative_to(args.modules_root)
                print(f"  DYNAMIC-MISSING {kind} {rel}:{line}: {value!r}")


if __name__ == "__main__":
    main()
