#!/usr/bin/env python3
"""
Generate GitHub Actions matrix entries from .github/build-config.json.
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path
from typing import Any


def load_config() -> dict[str, Any]:
    config_path = Path(__file__).resolve().parent.parent / "build-config.json"
    if not config_path.exists():
        raise FileNotFoundError(f"build-config.json not found at {config_path}")
    return json.loads(config_path.read_text(encoding="utf-8"))


def generate_matrix(
    config: dict[str, Any],
    platform: str,
    build_type: str | None = None,
    event_name: str | None = None,
    variant_filter: str | None = None,
) -> dict[str, Any]:
    platforms = config.get("platforms", {})
    if platform not in platforms:
        raise ValueError(f"Unknown platform '{platform}'")

    platform_cfg = platforms[platform]
    variants = platform_cfg.get("variants", [])
    if not variants:
        raise ValueError(f"No variants configured for '{platform}'")

    include: list[dict[str, Any]] = []

    for variant in variants:
        if variant_filter and variant.get("name") != variant_filter:
            continue

        variant_build_types = variant.get("build_types", ["Release"])
        if build_type:
            build_types = [bt for bt in variant_build_types if bt == build_type]
            if not build_types:
                continue
        elif event_name == "pull_request":
            # Keep PR cost down: prefer Debug if available, otherwise fallback to first defined type.
            build_types = ["Debug"] if "Debug" in variant_build_types else [variant_build_types[0]]
        else:
            build_types = variant_build_types

        for bt in build_types:
            entry: dict[str, Any] = {
                "name": variant.get("name", "default"),
                "os": variant["runner"],
                "host": variant["host"],
                "arch": variant["arch"],
                "build_type": bt,
            }

            for key in ("package", "qt_version", "qt_host", "qt_host_path", "shell", "primary", "abis", "gstreamer", "emulator"):
                if key in variant:
                    entry[key] = variant[key]

            include.append(entry)

    if not include:
        print(
            f"Warning: No matrix entries generated for platform '{platform}' "
            f"with filters build_type={build_type}, variant={variant_filter}",
            file=sys.stderr,
        )

    return {
        "include": include,
        "timeout_minutes": platform_cfg.get("timeout_minutes", 120),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate matrix entries for GitHub Actions.")
    parser.add_argument("--platform", required=True, choices=["linux", "windows", "macos", "android", "ios"])
    parser.add_argument("--build-type", default="")
    parser.add_argument("--event-name", default="")
    parser.add_argument("--variant", default="")
    parser.add_argument("--output", choices=["json", "github"], default="json")
    args = parser.parse_args()

    try:
        config = load_config()
        matrix = generate_matrix(
            config=config,
            platform=args.platform,
            build_type=args.build_type or None,
            event_name=args.event_name or None,
            variant_filter=args.variant or None,
        )
    except (FileNotFoundError, ValueError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    if args.output == "github":
        github_output = os.environ.get("GITHUB_OUTPUT")
        payload = json.dumps(matrix["include"])
        timeout = str(matrix["timeout_minutes"])
        if github_output:
            with open(github_output, "a", encoding="utf-8") as f:
                f.write(f"matrix={payload}\n")
                f.write(f"timeout_minutes={timeout}\n")
        else:
            print(f"matrix={payload}")
            print(f"timeout_minutes={timeout}")
    else:
        print(json.dumps(matrix, indent=2))

    return 0


if __name__ == "__main__":
    sys.exit(main())
