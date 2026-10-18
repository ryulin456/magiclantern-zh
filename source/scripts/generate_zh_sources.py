#!/usr/bin/env python3
"""Generate C resources from the known-good 5D3 Chinese localization binary."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from analyze_ml_binaries import find_payload, read_symbols
from core_translations_zh_cn import CORE_TRANSLATIONS
from extract_translation_dictionary import find_table
from module_metadata_translations_zh_cn import MODULE_METADATA_TRANSLATIONS
from find_translation_table import CJK_RE, resolve_string
from module_translations_zh_cn import MODULE_TRANSLATIONS


# Strings compiled only for the 6D target and therefore absent from the 5D3
# reference binary. Keep the source text here so the same FNV lookup path is
# used by both common and model-specific menu entries.
EXTRA_TRANSLATIONS = (
    ("GPS Power Save", "GPS 省电"),
    (
        "Disable the internal GPS at power off to save power.",
        "关机时禁用内置 GPS 以节省电量。",
    ),
    (
        "GPS is turned off from Canon menu, nothing to do.",
        "佳能菜单中的 GPS 已关闭，无需处理。",
    ),
    (
        "GPS set to external; this feature is only for the internal one.",
        "GPS 已设为外置；此功能仅适用于内置 GPS。",
    ),
    (
        "Internal GPS will be re-enabled at next startup.",
        "内置 GPS 将在下次启动时重新启用。",
    ),
    (
        "GPS broken (unknown state 0x%x)",
        "GPS 状态异常（未知状态 0x%x）",
    ),
)


def fnv1a(value: str) -> int:
    result = 0x811C9DC5
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 0x01000193) & 0xFFFFFFFF
    return result


def add_extra_translations(
    rows: list[tuple[int, int, str]],
) -> list[tuple[int, int, str]]:
    by_hash = {hash_value: (address, translated) for address, hash_value, translated in rows}
    explicit_sources = {
        source
        for source, _ in (
            EXTRA_TRANSLATIONS
            + MODULE_TRANSLATIONS
            + CORE_TRANSLATIONS
            + MODULE_METADATA_TRANSLATIONS
        )
    }
    legacy_metadata = tuple(
        (source.rstrip("\n"), translated)
        for source, translated in MODULE_METADATA_TRANSLATIONS
        if source.endswith("\n") and source.rstrip("\n") not in explicit_sources
    ) + (
        (
            'Records 14-bit RAW video in MLV format (with metadata).\n\n'
            'You may want to load mlv_snd for recording sound and mlv_play for reviewing clips.\n\n'
            'For details, please see the "Raw Video" section on the Magic Lantern forum.',
            '以 MLV 格式录制带元数据的 14 位 RAW 视频。可加载 mlv_snd 录制声音，并加载 mlv_play 在相机中查看片段。',
        ),
    )
    for source, translated in (
        EXTRA_TRANSLATIONS
        + MODULE_TRANSLATIONS
        + CORE_TRANSLATIONS
        + MODULE_METADATA_TRANSLATIONS
        + legacy_metadata
    ):
        hash_value = fnv1a(source)
        previous = by_hash.get(hash_value)
        if previous and previous[1] != translated:
            raise SystemExit(f"translation hash collision for {source!r}")
        by_hash[hash_value] = (0, translated)
    return sorted(
        ((address, hash_value, translated) for hash_value, (address, translated) in by_hash.items()),
        key=lambda row: row[1],
    )


def c_string(value: str) -> str:
    escaped = (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\r", "\\r")
        .replace("\n", "\\n")
        .replace("\t", "\\t")
    )
    return f'"{escaped}"'


def find_module_texts(
    blob: bytes,
    payload: int,
    restart_start: int,
) -> list[tuple[str, str, str]]:
    best: list[tuple[str, str, str]] = []
    for start in range(payload, len(blob) - 11, 4):
        rows: list[tuple[str, str, str]] = []
        for offset in range(start, len(blob) - 11, 12):
            name_address, summary_address, description_address = struct.unpack_from(
                "<III", blob, offset
            )
            name = resolve_string(blob, name_address, payload, restart_start)
            summary = resolve_string(blob, summary_address, payload, restart_start)
            description = resolve_string(
                blob, description_address, payload, restart_start
            )
            if (
                not name
                or len(name) > 8
                or CJK_RE.search(name)
                or not summary
                or not CJK_RE.search(summary)
                or not description
                or not CJK_RE.search(description)
            ):
                break
            rows.append((name, summary, description))
        if len(rows) > len(best):
            best = rows
    return best


def write_translation_file(
    path: Path,
    rows: list[tuple[int, int, str]],
) -> None:
    """Write translations outside autoexec.bin to protect the 6D memory map."""
    pool = bytearray()
    entries = []
    for _, hash_value, translated in rows:
        offset = len(pool)
        pool.extend(translated.encode("utf-8") + b"\0")
        entries.append((hash_value, offset))

    header_size = 20
    entry_size = 8
    pool_offset = header_size + entry_size * len(entries)
    header = struct.pack(
        "<4sIIII", b"MLZH", 1, len(entries), pool_offset, len(pool)
    )
    table = b"".join(struct.pack("<II", *entry) for entry in entries)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + table + pool)


def write_module_header(
    path: Path,
    rows: list[tuple[str, str, str]],
) -> None:
    lines = [
        "/* Text-only data imported from the reviewed 5D3 zh-CN catalog.",
        " * No 5D3 executable code, addresses, module binaries or memory layout are used. */",
        "#ifndef ML_ZH_CN_MODULES_H",
        "#define ML_ZH_CN_MODULES_H",
        "",
        "typedef struct",
        "{",
        "    const char *module_name;",
        "    const char *summary;",
        "    const char *description;",
        "} zh_cn_module_text_t;",
        "",
        "static const zh_cn_module_text_t zh_cn_module_texts[] =",
        "{",
    ]
    for name, summary, description in rows:
        lines.append(
            f"    {{ {c_string(name)}, {c_string(summary)}, {c_string(description)} }},"
        )
    lines.extend(
        [
            "    { 0, 0, 0 },",
            "};",
            "",
            "#endif",
            "",
        ]
    )
    path.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("autoexec", type=Path)
    parser.add_argument("sym", type=Path)
    parser.add_argument("restart_start", type=lambda value: int(value, 0))
    parser.add_argument("source_root", type=Path)
    args = parser.parse_args()

    blob = args.autoexec.read_bytes()
    symbols = read_symbols(args.sym)
    payload = find_payload(blob, symbols, args.restart_start)
    translations = find_table(blob, payload, args.restart_start)
    modules = find_module_texts(blob, payload, args.restart_start)

    if len(translations) != 1166:
        raise SystemExit(f"expected 1166 translations, found {len(translations)}")
    if len(modules) != 19:
        raise SystemExit(f"expected 19 module descriptions, found {len(modules)}")
    if any("\ufffd" in value for _, _, value in translations):
        raise SystemExit("replacement character found in translation table")

    translations = add_extra_translations(translations)

    write_translation_file(
        args.source_root / "data" / "fonts" / "zh_cn.lng", translations
    )
    write_module_header(args.source_root / "src" / "zh_cn_modules.h", modules)
    print(f"wrote {len(translations)} translations and {len(modules)} module records")


if __name__ == "__main__":
    main()
