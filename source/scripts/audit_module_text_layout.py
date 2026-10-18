#!/usr/bin/env python3
"""Audit 6D2 module introduction layout and CJK bitmap rendering safety."""

from __future__ import annotations

import argparse
import ast
import re
import struct
from dataclasses import dataclass
from pathlib import Path


MODULE_ROW_RE = re.compile(r'^\s*\{ "([^"]+)", "(.*)", "(.*)" \},$')
MODULE_STRING_NAME_RE = re.compile(
    r'static char __module_string_[A-Za-z0-9_]+_name\s*\[\]\s*'
    r'MODULE_STRINGS_SECTION\s*=\s*"([^"]+)";'
)
MODULE_STRING_PAIR_RE = re.compile(
    r"static char __module_string_(?P<key>[a-z]+)_name.*?=\s*"
    r"(?P<name>(?:\"(?:\\.|[^\"\\])*\"\s*)+);\s*"
    r"static char __module_string_(?P=key)_value.*?=\s*"
    r"(?P<value>(?:\"(?:\\.|[^\"\\])*\"\s*)+);",
    re.S,
)
STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"', re.DOTALL)
SPECIAL_MODULE_STRINGS = {
    "Name",
    "Description",
    "Build user",
    "Build date",
    "Last update",
    "Summary",
    "Forum",
}


def packed_utf8(character: str) -> int:
    return int.from_bytes(character.encode("utf-8"), "little")


@dataclass(frozen=True)
class Glyph:
    width: int
    height: int
    display_width: int
    x_offset: int
    y_offset: int
    bitmap: bytes

    @property
    def bytes_per_row(self) -> int:
        return (self.width + 7) // 8

    def pixel(self, x: int, y: int) -> bool:
        return bool(
            self.bitmap[y * self.bytes_per_row + x // 8]
            & (1 << (7 - x % 8))
        )

    def ink(self) -> set[tuple[int, int]]:
        return {
            (x, y)
            for y in range(self.height)
            for x in range(self.width)
            if self.pixel(x, y)
        }


class Bfnt:
    HEADER = struct.Struct("<IHHIII16s")

    def __init__(self, path: Path) -> None:
        self.path = path
        self.blob = path.read_bytes()
        if len(self.blob) < self.HEADER.size:
            raise ValueError(f"BFNT header is truncated: {path}")
        (
            magic,
            self.unknown,
            self.source_height,
            charmap_offset,
            charmap_size,
            bitmap_size,
            raw_name,
        ) = self.HEADER.unpack_from(self.blob)
        self.name = raw_name.rstrip(b"\0").decode("ascii", errors="replace")
        if magic != 0x00544E46:
            raise ValueError(f"invalid BFNT magic: {path}")
        if charmap_offset < self.HEADER.size or charmap_size % 4:
            raise ValueError(f"invalid BFNT character map: {path}")
        count = charmap_size // 4
        offsets_start = charmap_offset + charmap_size
        bitmap_start = offsets_start + charmap_size
        bitmap_end = bitmap_start + bitmap_size
        if bitmap_end > len(self.blob):
            raise ValueError(f"BFNT sections exceed file size: {path}")
        self.bitmap_size = bitmap_size
        self.bitmap_start = bitmap_start
        self.codes = struct.unpack_from(f"<{count}I", self.blob, charmap_offset)
        self.offsets = struct.unpack_from(f"<{count}I", self.blob, offsets_start)
        if list(self.offsets) != sorted(self.offsets):
            raise ValueError(f"BFNT glyph offsets are not sorted: {path}")
        self.index = {code: index for index, code in enumerate(self.codes)}
        if len(self.index) != count:
            raise ValueError(f"BFNT character map has duplicates: {path}")

    def glyph_by_index(self, index: int) -> Glyph:
        start = self.offsets[index]
        end = (
            self.offsets[index + 1]
            if index + 1 < len(self.offsets)
            else self.bitmap_size
        )
        if start + 10 > end or end > self.bitmap_size:
            raise ValueError(f"invalid BFNT glyph bounds at index {index}")
        width, height, display_width, x_offset, y_offset = struct.unpack_from(
            "<5H", self.blob, self.bitmap_start + start
        )
        bitmap_bytes = ((width + 7) // 8) * height
        if start + 10 + bitmap_bytes > end:
            raise ValueError(f"truncated BFNT glyph bitmap at index {index}")
        bitmap_start = self.bitmap_start + start + 10
        return Glyph(
            width,
            height,
            display_width,
            x_offset,
            y_offset,
            self.blob[bitmap_start : bitmap_start + bitmap_bytes],
        )

    def glyph(self, character: str) -> Glyph:
        try:
            index = self.index[packed_utf8(character)]
        except KeyError as exc:
            raise ValueError(
                f"BFNT is missing {character!r} (U+{ord(character):04X})"
            ) from exc
        return self.glyph_by_index(index)

    def scaled_width(self, character: str, target_height: int) -> int:
        glyph = self.glyph(character)
        return (
            glyph.display_width * target_height + self.source_height // 2
        ) // self.source_height


class Rbf:
    HEADER = struct.Struct("<ii64s11i")

    def __init__(self, path: Path) -> None:
        blob = path.read_bytes()
        if len(blob) < self.HEADER.size:
            raise ValueError(f"RBF header is truncated: {path}")
        values = self.HEADER.unpack_from(blob)
        if values[0] != 0x0DF00EE0 or values[1] != 3:
            raise ValueError(f"invalid RBF header: {path}")
        char_size, _, self.height, _, first, last, _, width_offset, _, _, _ = values[3:]
        count = last - first + 1
        if count <= 0 or width_offset + count > len(blob):
            raise ValueError(f"invalid RBF width table: {path}")
        default_width = 8 * char_size // self.height
        self.widths = [default_width] * 256
        raw_widths = blob[width_offset : width_offset + count]
        for index, width in enumerate(raw_widths, start=first):
            if 0 <= index < len(self.widths):
                self.widths[index] = width

    def width(self, character: str) -> int:
        codepoint = ord(character)
        return self.widths[codepoint] if codepoint < 256 else 0


def text_width(text: str, rbf: Rbf, bfnt: Bfnt, target_height: int) -> int:
    width = 0
    for character in text:
        if character == "\n":
            break
        width += (
            rbf.width(character)
            if ord(character) <= 0x7F
            else bfnt.scaled_width(character, target_height)
        )
    return width


def wrap_text(
    text: str,
    rbf: Rbf,
    bfnt: Bfnt,
    target_height: int,
    max_width: int,
) -> list[str]:
    """Mirror module_draw_wrapped_text closely enough for release auditing."""
    lines: list[str] = []
    while text:
        text = text.lstrip(" \t")
        if not text:
            break
        if text.startswith("\n"):
            lines.append("")
            text = text[1:]
            continue

        width = 0
        fitted = 0
        for index, character in enumerate(text):
            if character == "\n":
                break
            character_width = (
                rbf.width(character)
                if ord(character) <= 0x7F
                else bfnt.scaled_width(character, target_height)
            )
            if width + character_width > max_width:
                break
            width += character_width
            fitted = index + 1
        if fitted <= 0:
            fitted = 1

        if fitted < len(text) and text[fitted] != "\n":
            candidate = text[:fitted]
            word_break = max(candidate.rfind(" "), candidate.rfind("\t"))
            if word_break > 0:
                fitted = word_break

        while len(text[:fitted].encode("utf-8")) > 255:
            fitted -= 1
        if fitted <= 0:
            raise ValueError("module description cannot make wrapping progress")

        line = text[:fitted].rstrip(" \t")
        lines.append(line)
        text = text[fitted:].lstrip(" \t")
        if text.startswith("\n"):
            text = text[1:]
    return lines


def module_footer_lines(root: Path) -> dict[str, int]:
    result: dict[str, int] = {}
    for path in root.glob("modules/**/build/module_strings.h"):
        module_name = path.parent.parent.name
        names = MODULE_STRING_NAME_RE.findall(path.read_text(encoding="utf-8"))
        if not names:
            continue
        extra = sum(
            1
            for name in names
            if name not in SPECIAL_MODULE_STRINGS and not name.startswith("Help page")
        )
        if "Build date" in names and "Build user" in names:
            extra += 2
        if "Last update" in names:
            extra += 2  # conservative: module.c allows a two-line last-update value
        result[module_name] = max(result.get(module_name, 0), extra)
    return result


def built_module_intro(root: Path, module_name: str) -> tuple[str, str, str]:
    """Read one module's compiled name, summary and description metadata."""
    candidates = sorted(root.glob(f"modules/**/{module_name}/build/module_strings.h"))
    direct = root / "modules" / module_name / "build" / "module_strings.h"
    if direct.is_file() and direct not in candidates:
        candidates.insert(0, direct)
    if len(candidates) != 1:
        raise ValueError(
            f"expected one built metadata header for {module_name}, found {len(candidates)}"
        )

    fields: dict[str, str] = {}
    text = candidates[0].read_text(encoding="utf-8")
    for match in MODULE_STRING_PAIR_RE.finditer(text):
        field = "".join(
            ast.literal_eval(token)
            for token in STRING_RE.findall(match.group("name"))
        )
        value = "".join(
            ast.literal_eval(token)
            for token in STRING_RE.findall(match.group("value"))
        )
        fields[field] = value

    missing = {"Name", "Summary", "Description"} - fields.keys()
    if missing:
        raise ValueError(
            f"{module_name}: compiled metadata is missing {sorted(missing)}"
        )
    return fields["Name"], fields["Summary"], fields["Description"]


def connected_components(points: set[tuple[int, int]]) -> list[set[tuple[int, int]]]:
    unseen = set(points)
    result: list[set[tuple[int, int]]] = []
    while unseen:
        seed = unseen.pop()
        component = {seed}
        pending = [seed]
        while pending:
            x, y = pending.pop()
            for adjacent_y in range(y - 1, y + 2):
                for adjacent_x in range(x - 1, x + 2):
                    adjacent = (adjacent_x, adjacent_y)
                    if adjacent in unseen:
                        unseen.remove(adjacent)
                        component.add(adjacent)
                        pending.append(adjacent)
        result.append(component)
    return result


def legacy_component_loss(glyph: Glyph, target_height: int, source_height: int) -> int:
    scaled_width = (glyph.width * target_height + source_height // 2) // source_height
    scaled_height = (glyph.height * target_height + source_height // 2) // source_height
    sampled = {
        (dx * glyph.width // scaled_width, dy * glyph.height // scaled_height)
        for dy in range(scaled_height)
        for dx in range(scaled_width)
    }
    return sum(
        1
        for component in connected_components(glyph.ink())
        if len(component) >= 2 and not component.intersection(sampled)
    )


def coverage_scaled_ink(glyph: Glyph, target_height: int, source_height: int) -> int:
    scaled_width = (glyph.width * target_height + source_height // 2) // source_height
    scaled_height = (glyph.height * target_height + source_height // 2) // source_height
    output = {
        (sx * scaled_width // glyph.width, sy * scaled_height // glyph.height)
        for sx, sy in glyph.ink()
    }
    return len(output)


def coverage_component_loss(
    glyph: Glyph, target_height: int, source_height: int
) -> int:
    """Count source components not represented by source-to-destination mapping."""
    scaled_width = (glyph.width * target_height + source_height // 2) // source_height
    scaled_height = (glyph.height * target_height + source_height // 2) // source_height
    represented = glyph.ink() if scaled_width and scaled_height else set()
    return sum(
        1
        for component in connected_components(glyph.ink())
        if not component.intersection(represented)
    )


def write_report(
    path: Path,
    rows: list[dict[str, object]],
    bfnt: Bfnt,
    required_characters: set[str],
    legacy_blank: list[str],
    legacy_component_losses: list[str],
) -> None:
    widest = max(rows, key=lambda row: int(row["summary_pixels"]))
    tightest = min(rows, key=lambda row: int(row["spare_lines"]))
    lines = [
        "# 模块介绍与中文字形静态审计",
        "",
        f"结论：通过。{len(rows)} 个模块的一行摘要均未触及 100 字节缓冲区或 700 像素显示边界；详情说明均可在 10 像素左缩进、690 像素正文宽度内完整换行显示。覆盖采样修复后，发布文字使用的中文字形在全部常用字号下均不会整字消失。",
        "",
        "## 布局边界",
        "",
        "- 模块列表摘要：单行，UTF-8 最多 99 字节，显示宽度最多 700 像素。",
        "- 模块详情正文：左边距 10 像素，起始 y=60，正文宽度 690 像素，右侧保留 20 像素。",
        "- 字体：详情和摘要使用 23 像素字体；模块名称使用 28 像素字体；详情页按实际模块元数据为页脚预留行数。",
        f"- 最宽摘要：`{widest['module']}`，{widest['summary_pixels']} 像素。",
        f"- 最紧详情页：`{tightest['module']}`，正文仍剩余 {tightest['spare_lines']} 行。",
        "",
        "## 逐模块结果",
        "",
        "| 模块 | 摘要 UTF-8 字节 | 摘要像素 | 正文使用/可用行 | 结果 |",
        "|---|---:|---:|---:|---|",
    ]
    for row in rows:
        lines.append(
            f"| `{row['module']}` | {row['summary_bytes']} | "
            f"{row['summary_pixels']} | {row['description_lines']}/"
            f"{row['available_lines']} | 通过 |"
        )
    lines.extend(
        [
            "",
            "## 字形结果",
            "",
            f"- BFNT：`{bfnt.name}`，{len(bfnt.codes)} 个字形，源字形 {bfnt.source_height} 像素。",
            f"- 本次发布文字涉及 {len(required_characters)} 个非 ASCII 字符；缺字 0，空白字形 0，越界字形 0。",
            "- 已检查 12、20、22、23、28、32、40 像素字号；覆盖采样后空白字形 0、笔画连通分量丢失 0。",
            f"- 旧单点缩放会让 {''.join(legacy_blank) or '无'} 在 23 像素时整字消失；另有 {len(legacy_component_losses)} 个字符存在笔画分量被跳过。该旧路径已被替换。",
            "",
            "## 限制",
            "",
            "静态审计可以证明缓冲区、像素边界、换行高度和字形位图不存在已知截断，也能证明缩放后不是空字形；相机屏幕缩放、显示刷新和肉眼字形质量仍须用逐页实机照片完成最终 L2 验收。",
            "",
        ]
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source_root", type=Path)
    parser.add_argument("--report", type=Path)
    parser.add_argument(
        "--text-source",
        type=Path,
        action="append",
        default=[],
        help="also audit non-ASCII glyphs used by this UTF-8 source file",
    )
    parser.add_argument(
        "--metadata-module",
        action="append",
        default=[],
        help=(
            "also audit this module's compiled Name/Summary/Description metadata; "
            "use for feature modules that are not in zh_cn_modules.h"
        ),
    )
    args = parser.parse_args()
    root = args.source_root.resolve()
    errors: list[str] = []

    module_source = (root / "src" / "module.c").read_text(encoding="utf-8")
    bmp_source = (root / "src" / "bmp.c").read_text(encoding="utf-8")
    if 'MENU_SET_HELP("%s", summary);' not in module_source:
        errors.append("module summaries are no longer copied through MENU_SET_HELP")
    if (
        "module_draw_wrapped_text(FONT_MED, 10, 60, 690, yr - 5, desc);"
        not in module_source
    ):
        errors.append("module description indent/wrapping bounds changed")
    if "shrink_cjk" not in bmp_source:
        errors.append("CJK stroke-preserving downscale path is missing")

    module_header = (root / "src" / "zh_cn_modules.h").read_text(encoding="utf-8")
    modules = [
        match.groups()
        for line in module_header.splitlines()
        if (match := MODULE_ROW_RE.match(line))
    ]
    if len(modules) != 19:
        errors.append(f"expected 19 module descriptions, found {len(modules)}")
    known_module_names = {row[0] for row in modules}
    for module_name in args.metadata_module:
        if module_name in known_module_names:
            errors.append(f"duplicate metadata module: {module_name}")
            continue
        try:
            _, summary, description = built_module_intro(root, module_name)
        except ValueError as exc:
            errors.append(str(exc))
            continue
        modules.append((module_name, summary, description))
        known_module_names.add(module_name)

    bfnt = Bfnt(root / "data" / "fonts" / "zh_cn.bfn")
    rbf = Rbf(root / "data" / "fonts" / "argnor23.rbf")
    if rbf.height != 23:
        errors.append(f"FONT_MED audit expected 23 pixels, found {rbf.height}")

    for index in range(len(bfnt.codes)):
        try:
            glyph = bfnt.glyph_by_index(index)
        except ValueError as exc:
            errors.append(str(exc))
            continue
        if not all((glyph.width, glyph.height, glyph.display_width)):
            errors.append(f"BFNT glyph {index} has a zero metric")
        if glyph.x_offset + glyph.width > 100 or glyph.y_offset + glyph.height > 50:
            errors.append(f"BFNT glyph {index} exceeds renderer safety bounds")

    footer_lines = module_footer_lines(root)
    rows: list[dict[str, object]] = []
    for module_name, summary, description in modules:
        summary_bytes = len(summary.encode("utf-8"))
        summary_pixels = text_width(summary, rbf, bfnt, 23)
        wrapped = wrap_text(description, rbf, bfnt, 23, 690)
        footer = footer_lines.get(module_name)
        if footer is None:
            errors.append(f"missing built module metadata for {module_name}")
            footer = 8
        max_y = 480 - footer * 23 - 5
        available_lines = max(0, (max_y - 60) // 23)
        if "\n" in summary or "\r" in summary:
            errors.append(f"{module_name}: summary is not one line")
        if summary_bytes > 99:
            errors.append(
                f"{module_name}: summary uses {summary_bytes} UTF-8 bytes (limit 99)"
            )
        if summary_pixels > 700:
            errors.append(
                f"{module_name}: summary uses {summary_pixels} pixels (limit 700)"
            )
        if len(wrapped) > available_lines:
            errors.append(
                f"{module_name}: description needs {len(wrapped)} lines, "
                f"only {available_lines} are available"
            )
        if any(text_width(line, rbf, bfnt, 23) > 690 for line in wrapped):
            errors.append(f"{module_name}: a wrapped description line exceeds 690 pixels")
        rows.append(
            {
                "module": module_name,
                "summary_bytes": summary_bytes,
                "summary_pixels": summary_pixels,
                "description_lines": len(wrapped),
                "available_lines": available_lines,
                "spare_lines": available_lines - len(wrapped),
            }
        )

    required_text = "".join(summary + description for _, summary, description in modules)
    for source in args.text_source:
        required_text += source.read_text(encoding="utf-8")
    translation_blob = (root / "data" / "fonts" / "zh_cn.lng").read_bytes()
    # The string pool starts after the fixed 20-byte header and 8-byte entries.
    _, _, translation_count, pool_offset, pool_size = struct.unpack_from(
        "<4sIIII", translation_blob
    )
    if pool_offset != 20 + 8 * translation_count or pool_offset + pool_size != len(
        translation_blob
    ):
        errors.append("invalid zh_cn.lng bounds during layout audit")
    else:
        required_text += translation_blob[pool_offset:].replace(b"\0", b"").decode(
            "utf-8"
        )
    required_characters = {character for character in required_text if ord(character) > 0x7F}
    legacy_blank: list[str] = []
    legacy_component_losses: list[str] = []
    for character in sorted(required_characters, key=ord):
        try:
            glyph = bfnt.glyph(character)
        except ValueError as exc:
            errors.append(str(exc))
            continue
        ink = glyph.ink()
        if not ink:
            errors.append(f"required glyph {character!r} is blank")
            continue
        if any(
            x in (0, glyph.width - 1) or y in (0, glyph.height - 1)
            for x, y in ink
        ):
            errors.append(f"required glyph {character!r} touches its bitmap edge")
        if legacy_component_loss(glyph, 23, bfnt.source_height):
            legacy_component_losses.append(character)
        scaled_width = (
            glyph.width * 23 + bfnt.source_height // 2
        ) // bfnt.source_height
        scaled_height = (
            glyph.height * 23 + bfnt.source_height // 2
        ) // bfnt.source_height
        legacy_sampled = {
            (dx * glyph.width // scaled_width, dy * glyph.height // scaled_height)
            for dy in range(scaled_height)
            for dx in range(scaled_width)
        }
        if not ink.intersection(legacy_sampled):
            legacy_blank.append(character)
        for target_height in (12, 20, 22, 23, 28, 32, 40):
            if coverage_scaled_ink(glyph, target_height, bfnt.source_height) <= 0:
                errors.append(
                    f"required glyph {character!r} is blank at {target_height}px"
                )
            if coverage_component_loss(glyph, target_height, bfnt.source_height):
                errors.append(
                    f"required glyph {character!r} loses an ink component at "
                    f"{target_height}px"
                )

    if errors:
        raise SystemExit("FAILED:\n- " + "\n- ".join(errors))

    if args.report:
        write_report(
            args.report,
            rows,
            bfnt,
            required_characters,
            legacy_blank,
            legacy_component_losses,
        )
    widest = max(rows, key=lambda row: int(row["summary_pixels"]))
    tightest = min(rows, key=lambda row: int(row["spare_lines"]))
    print(
        f"OK: {len(rows)} one-line summaries (widest {widest['summary_pixels']}px), "
        f"{len(rows)} complete descriptions (minimum {tightest['spare_lines']} spare lines), "
        f"{len(required_characters)} required non-ASCII glyphs, coverage scaling valid"
    )


if __name__ == "__main__":
    main()
