#!/usr/bin/env python3
"""Static guardrails for the 5D3 native-AF experimental module."""

import sys
from pathlib import Path


def require(source: str, needle: str, label: str, errors: list[str]) -> None:
    if needle not in source:
        errors.append(f"missing {label}: {needle}")


def main() -> None:
    path = Path(sys.argv[1]) if len(sys.argv) == 2 else Path("modules/vid_af/vid_af.c")
    source = path.read_text(encoding="utf-8")
    errors: list[str] = []
    for needle, label in (
        ('is_camera("5D3", "1.2.3")', "exact camera/firmware guard"),
        ("get_afframe_pos(720, 480, &x, &y)", "Canon AF-frame read"),
        ("VIDAF_MOVE_DEBOUNCE_MS", "move debounce"),
        ("!is_movie_mode() || RECORDING", "movie-mode and no-recording guard"),
        ("lens_setup_af(AF_ENABLE)", "native AF setup"),
        ("MODULE_KEY_PRESS_HALFSHUTTER", "native SW1 press"),
        ("MODULE_KEY_UNPRESS_HALFSHUTTER", "native SW1 release"),
        ("lens_cleanup_af()", "AF-setting restoration"),
        ("VIDAF_START_TIMEOUT_MS", "start timeout"),
        ("VIDAF_DONE_TIMEOUT_MS", "completion timeout"),
    ):
        require(source, needle, label, errors)

    forbidden = (
        "lens_focus(",
        "prop_request_change(",
        "prop_request_change_wait(",
        "call(",
        "MEM(",
        "shutter",
        "SW2(",
    )
    for needle in forbidden:
        if needle in source:
            errors.append(f"forbidden direct hardware/action call: {needle}")

    if source.count("MODULE_KEY_PRESS_HALFSHUTTER") != 1:
        errors.append("native SW1 press must have exactly one call site")
    if source.count("MODULE_KEY_UNPRESS_HALFSHUTTER") != 1:
        errors.append("native SW1 release must have exactly one call site")

    if errors:
        raise SystemExit("\n".join(errors))
    print(
        "OK: exact 5D3.123 guard; Canon AF-frame debounce; native SW1 only; "
        "recording guard; bounded start/done waits; release and AF-setting restore; "
        "no raw lens-drive, direct property, ROM, shutter or register calls"
    )


if __name__ == "__main__":
    main()
