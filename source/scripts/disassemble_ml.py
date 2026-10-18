#!/usr/bin/env python3
"""Disassemble selected ARM regions from a Magic Lantern autoexec payload."""

from __future__ import annotations

import argparse
import bisect
import os
import struct
import sys
from pathlib import Path

sys.path.insert(0, os.environ.get("ML_PYDEPS", ""))
from capstone import (  # type: ignore  # noqa: E402
    CS_ARCH_ARM,
    CS_MODE_ARM,
    CS_MODE_LITTLE_ENDIAN,
    Cs,
)
from capstone.arm import ARM_OP_IMM, ARM_OP_MEM, ARM_REG_PC  # type: ignore  # noqa: E402

from analyze_ml_binaries import find_payload, read_symbols  # noqa: E402


def nearest_symbol(address: int, ordered: list[tuple[int, str]]) -> str:
    addresses = [item[0] for item in ordered]
    index = bisect.bisect_right(addresses, address) - 1
    if index < 0:
        return "?"
    base, name = ordered[index]
    return name if address == base else f"{name}+0x{address - base:x}"


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("autoexec", type=Path)
    parser.add_argument("sym", type=Path)
    parser.add_argument("restart_start", type=lambda value: int(value, 0))
    parser.add_argument("--symbol")
    parser.add_argument("--address", type=lambda value: int(value, 0))
    parser.add_argument("--bytes", type=lambda value: int(value, 0), default=0x200)
    parser.add_argument("--calls-to")
    parser.add_argument("--calls-address", type=lambda value: int(value, 0))
    parser.add_argument("--scan-bytes", type=lambda value: int(value, 0), default=0x60000)
    args = parser.parse_args()

    blob = args.autoexec.read_bytes()
    symbols = read_symbols(args.sym)
    ordered = sorted(
        (address, name)
        for name, address in symbols.items()
        if args.restart_start <= address < 0x10000000
    )
    reverse = {address: name for address, name in ordered}
    payload = find_payload(blob, symbols, args.restart_start)

    md = Cs(CS_ARCH_ARM, CS_MODE_ARM | CS_MODE_LITTLE_ENDIAN)
    md.detail = True
    md.skipdata = True

    def disassemble(start: int, size: int) -> None:
        offset = payload + start - args.restart_start
        code = blob[offset : offset + size]
        for instruction in md.disasm(code, start):
            comments: list[str] = []
            if instruction.mnemonic.startswith("b") and instruction.operands:
                operand = instruction.operands[0]
                if operand.type == ARM_OP_IMM:
                    comments.append(nearest_symbol(operand.imm, ordered))
            if instruction.mnemonic == "ldr" and len(instruction.operands) >= 2:
                operand = instruction.operands[1]
                if operand.type == ARM_OP_MEM and operand.mem.base == ARM_REG_PC:
                    literal_address = instruction.address + 8 + operand.mem.disp
                    literal_offset = payload + literal_address - args.restart_start
                    if 0 <= literal_offset <= len(blob) - 4:
                        value = struct.unpack_from("<I", blob, literal_offset)[0]
                        comments.append(
                            f"literal[0x{literal_address:08x}]=0x{value:08x} "
                            f"({nearest_symbol(value, ordered)})"
                        )
            suffix = f" ; {'; '.join(comments)}" if comments else ""
            print(
                f"0x{instruction.address:08x}: {instruction.mnemonic:8s} "
                f"{instruction.op_str}{suffix}"
            )

    if args.calls_to or args.calls_address is not None:
        target = (
            symbols.get(args.calls_to)
            if args.calls_to
            else args.calls_address
        )
        if target is None:
            raise SystemExit(f"unknown target symbol: {args.calls_to}")
        target_label = args.calls_to or f"0x{target:08x}"
        scan_size = min(args.scan_bytes, len(blob) - payload)
        for instruction in md.disasm(blob[payload : payload + scan_size], args.restart_start):
            if not instruction.mnemonic.startswith("bl") or not instruction.operands:
                continue
            operand = instruction.operands[0]
            if operand.type == ARM_OP_IMM and operand.imm == target:
                print(
                    f"0x{instruction.address:08x} "
                    f"{nearest_symbol(instruction.address, ordered)} -> {target_label}"
                )

    if args.symbol or args.address is not None:
        start = symbols[args.symbol] if args.symbol else args.address
        assert start is not None
        print(f"\n[{nearest_symbol(start, ordered)} @ 0x{start:08x}]")
        disassemble(start, args.bytes)


if __name__ == "__main__":
    main()
