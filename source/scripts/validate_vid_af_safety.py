#!/usr/bin/env python3
"""Static release guards for the experimental 5D3 video-AF module."""

from __future__ import annotations

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1] / "magiclantern-zh-src"
SOURCE = ROOT / "modules" / "vid_af" / "vid_af.c"
POLICY = ROOT / "platform" / "5D3.123" / "modules.included"


def compact(value: str) -> str:
    return re.sub(r"\s+", "", value)


def main() -> None:
    text = SOURCE.read_text(encoding="utf-8")
    errors: list[str] = []

    required = {
        "feature defaults off": 'CONFIG_INT("vidaf.enabled", vidaf_enabled, 0)',
        "exact model/firmware guard": 'is_camera("5D3", "1.2.3")',
        "task failure bit checked": "task_result & 1",
        "manual-focus rejection": "is_manual_focus()",
        "Canon continuous-AF rejection": "is_continuous_af()",
        "lens busy rejection": "lens_info.job_state != 0",
        "half-shutter cancellation": "get_halfshutter_pressed()",
        "post-sleep readiness check": (
            "msleep(VIDAF_FRAME_DELAY_MS);\n        /* LiveView or the selected mode may change while the task sleeps. */\n        if (!vidaf_ready()) return 0;"
        ),
        "failed-focus LED cleanup": "if (!ok) info_led_off();",
        "cancel key is consumed": (
            "vidaf_cancel = 1;\n            return 0;"
        ),
        "unload waits for worker": "vidaf_shutdown = 1",
    }
    for label, token in required.items():
        if token not in text:
            errors.append(f"missing {label}: {token}")

    focus_calls = re.findall(r"\blens_focus\s*\((.*?)\)", text, re.S)
    if len(focus_calls) != 1:
        errors.append(f"expected exactly one lens_focus call, found {len(focus_calls)}")
    elif compact(focus_calls[0]) != compact(
        "1, direction * step_size, 1, vidaf_settle_ms[settle_index]"
    ):
        errors.append("lens_focus must issue one command with wait=1")

    maximum = re.search(r"#define\s+VIDAF_MAX_COMMANDS\s+(\d+)", text)
    if not maximum or int(maximum.group(1)) > 40:
        errors.append("focus-command hard limit is absent or greater than 40")

    forbidden = (
        "prop_request_change(",
        "prop_request_change_wait(",
        "EngDrvOut(",
        "MEM(",
        "lens_take_picture(",
        "lens_take_pictures(",
    )
    for token in forbidden:
        if token in text:
            errors.append(f"forbidden direct hardware/action call: {token}")

    policy_names = {
        line.split("#", 1)[0].strip()
        for line in POLICY.read_text(encoding="utf-8").splitlines()
        if line.split("#", 1)[0].strip()
    }
    if "vid_af" not in policy_names:
        errors.append("vid_af is absent from the 5D3.123 package policy")

    if errors:
        raise SystemExit("FAILED:\n- " + "\n- ".join(errors))

    print(
        "OK: disabled by default; exact 5D3.123 guard; one serialized "
        "wait=1 lens command site; <=40 commands; post-sleep state check; "
        "failed-focus LED cleanup; cancel keys consumed; no direct property, "
        "ROM, shutter or low-level register calls"
    )


if __name__ == "__main__":
    main()
