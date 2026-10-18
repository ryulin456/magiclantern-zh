#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2025 Kajetan Krykwiński (@kitor)
# Added SCRIPT flag support on FAT16/32 and to remove already set flags.
# Implemented argparse, rewritten a bit for modern Python
#
# Copyright © 2013 Diego Elio Pettenò <flameeyes@flameeyes.eu>
# Inspired by Trammel's and arm.indiana@gmail.com's shell script
#
#         DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
#                     Version 2, December 2004
#
#  Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
#
#  Everyone is permitted to copy and distribute verbatim or modified
#  copies of this license document, and changing it is allowed as long
#  as the name is changed.
#
#             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
#    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
#
#   0. You just DO WHAT THE FUCK YOU WANT TO.
#
#
# patch the SD/CF card bootsector to make it bootable on Canon DSLR
# See http://chdk.setepontos.com/index.php/topic,4214.0.html
#
# Supports FAT16, FAT32 and EXFAT devices; should work on all
# operating systems as long as Numpy is available.

description = """
Write card flags.

To make card Magic Lantern / CHDK bootable, at least EOS_DEVELOP and BOOTDISK
has to be set. For Canon Basic scripting (CHDK, MagicLantern on Digic 8 and up)
SCRIPT flag has to be set too.

If only partition is provided, script defaults to setting all three flags.

Note that SCRIPT flag works only with FAT16/FAT32 formatted cards.

This is a design decision, as we found Canon code has a bug in handling
32GB and less EXFAT cards.
On cards above 32GB flag would go to "sector before partition start", while
on 32GB or less it is required in partition MBR at position that simply
destroys EXFAT filesystem.
Likely an oversight as Canon code will format only > 32G cards as EXFAT.

We recently found out that old cameras (tested on 70D) have problems with
booting from some EXFAT cards. 16G cards created on Linux worked fine on
77D and R, but bootloader failed to find autoexec.bin on 70D.
Which means camera sees flags fine but can't find the file. Yet the card
works fine in camera after flags removal. At the moment of writing
reason is unknown.
64GB EXFAT card formatted in camera worked fine. Possibly a bug in bootloader
implementation of exfat in older models.
"""

import platform
import sys
import mmap
import struct
import argparse
from argparse import RawDescriptionHelpFormatter
from dataclasses import dataclass
from numpy import uint8, uint32

@dataclass
class FAT16:
    eos_develop: int = 43
    bootdisk: int = 64
    script: int = 496

@dataclass
class FAT32:
    eos_develop: int = 71
    bootdisk: int = 92
    script: int = 496

@dataclass
class EXFAT:
    eos_develop: int = 130
    bootdisk: int = 122
    script: int = None

def setboot(dev, base, offsets, args):
    print('Applying parameters on device')
    if offsets.eos_develop and args.eos_develop:
        off = base + offsets.eos_develop
        print(f'  writing EOS_DEVELOP at offset {off:x} (Volume Label)')
        dev[off:off+11] = b'EOS_DEVELOP'
    if offsets.bootdisk and args.bootdisk:
        off = base + offsets.bootdisk
        print(f'  writing BOOTDISK at offset {off:x}  (Boot code)')
        dev[off:off+8] = b'BOOTDISK'
    if offsets.script and args.script:
        off = base + offsets.script
        print(f'  writing SCRIPT at offset {off:x}')
        dev[off:off+6] = b'SCRIPT'

    if not offsets.script and args.script:
       print("  Selected FS doesn't support SCRIPT flag, ommiting!")


    if args.remove_unused_flags:
        if offsets.eos_develop and not args.eos_develop:
            off = base + offsets.eos_develop
            print(f'  removing EOS_DEVELOP at offset {off:x} (Volume Label)')
            dev[off:off+11] = b'\0\0\0\0\0\0\0\0\0\0\0'
        if offsets.bootdisk and not args.bootdisk:
            off = base + offsets.bootdisk
            print(f'  removing BOOTDISK at offset {off:x} (Boot code)')
            dev[off:off+8] =  b'\0\0\0\0\0\0\0\0'
        if offsets.script and not args.script:
            off = base + offsets.script
            print(f'  removing SCRIPT at offset {off:x}')
            dev[off:off+6] = b'\0\0\0\0\0\0'


def setExfat(device, base, args):
    print(f'Updating EXFAT VBR at {base:x}')
    setboot(device, base, EXFAT, args)

    EXFAT_VBR_SIZE = 512 * 11

    print('Calculating VBR checksum')
    checksum = uint32(0)
    for index in range(0, EXFAT_VBR_SIZE):
        if index in {106, 107, 112}:
            continue
        value = uint8(device[base+index])
        checksum = uint32((checksum << 31) | (checksum >> 1) + value)

    checksum_chunk = struct.pack('<I', checksum) * 128
    device[base+512*11:base+512*12] = checksum_chunk


def main():
    parser = argparse.ArgumentParser(
                formatter_class = RawDescriptionHelpFormatter,
                description=description
            )
    parser.add_argument("partition",
            help="/dev path to first device partition")
    parser.add_argument("--eos-develop", action="store_true",
            help="Set EOS_DEVELOP flag")
    parser.add_argument("--bootdisk", action="store_true",
            help="Set BOOTDISK flag")
    parser.add_argument("--script", action="store_true",
            help="Set SCRIPT flag")
    parser.add_argument("--remove-unused-flags", action="store_true",
            help="Use to clear flags that are not selected")

    args = parser.parse_args()

    if not any([args.eos_develop, args.bootdisk,
            args.script, args.remove_unused_flags]):
        print('No flags selected. Defaulting to "set all flags, remove none"')
        args.eos_develop = True
        args.bootdisk = True
        args.script = True

    device_fd = open(args.partition, 'r+b')
    # Only map the longest boot recod we need to write to (512*24 is
    # needed for exfat)
    device = mmap.mmap(device_fd.fileno(), 512*24)

    if device[54:62] == b'FAT16   ':
        print('Identified FAT16 partition')
        setboot(device, 0, FAT16, args)
    elif device[82:90] == b'FAT32   ':
        print('Identified FAT32 partition')
        setboot(device, 0, FAT32, args)
    elif device[3:11] == b'EXFAT   ':
        print('Identified EXFAT partition')
        setExfat(device, 0, args)
        # backup VBR, set that one up as well
        setExfat(device, 512*12, args)
    else:
        print(f"""\
Device {args.partition} is not a valid filesystem in the supported range.
Supported filesystems are FAT16, FAT32, EXFAT.
Format your card in camera before using this script.""", file=sys.stderr)

    device.flush()
    device_fd.close()

if __name__ == '__main__':
    main()
