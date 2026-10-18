#!/usr/bin/env python3
"""Compare sections and symbols in two ELF objects."""

from __future__ import annotations

import argparse
import hashlib
import os
import sys
from pathlib import Path

sys.path.insert(0, os.environ.get("ML_PYDEPS", ""))
from elftools.elf.elffile import ELFFile  # type: ignore  # noqa: E402


def inspect(path: Path):
    with path.open("rb") as stream:
        elf = ELFFile(stream)
        sections = {
            section.name: (
                section["sh_type"],
                section["sh_size"],
                hashlib.sha256(section.data()).hexdigest()[:16]
                if section["sh_type"] != "SHT_NOBITS"
                else "nobits",
            )
            for section in elf.iter_sections()
        }
        symbols = {}
        symbol_table = elf.get_section_by_name(".symtab")
        if symbol_table:
            for symbol in symbol_table.iter_symbols():
                symbols[symbol.name] = (
                    symbol["st_shndx"],
                    symbol["st_value"],
                    symbol["st_size"],
                    symbol["st_info"]["bind"],
                    symbol["st_info"]["type"],
                )
        undefined = sorted(
            name for name, value in symbols.items() if value[0] == "SHN_UNDEF" and name
        )
        return sections, symbols, undefined


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("old", type=Path)
    parser.add_argument("new", type=Path)
    args = parser.parse_args()

    old_sections, old_symbols, old_undefined = inspect(args.old)
    new_sections, new_symbols, new_undefined = inspect(args.new)

    print("[sections]")
    for name in sorted(set(old_sections) | set(new_sections)):
        if old_sections.get(name) != new_sections.get(name):
            print(f"{name}: {old_sections.get(name)} -> {new_sections.get(name)}")

    print("\n[undefined symbols]")
    print("added:", sorted(set(new_undefined) - set(old_undefined)))
    print("removed:", sorted(set(old_undefined) - set(new_undefined)))

    print("\n[changed/added/removed symbols]")
    for name in sorted(set(old_symbols) | set(new_symbols)):
        if old_symbols.get(name) != new_symbols.get(name):
            print(f"{name}: {old_symbols.get(name)} -> {new_symbols.get(name)}")


if __name__ == "__main__":
    main()
