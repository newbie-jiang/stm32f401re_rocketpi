#!/usr/bin/env python3
"""
Parse the ARM MDK map file and report the largest Flash and RAM users.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import List, Dict


def parse_component_table(map_path: Path) -> List[Dict[str, int]]:
    lines = map_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    try:
        start = next(i for i, line in enumerate(lines) if "Image component sizes" in line)
    except StopIteration as exc:  # pragma: no cover - defensive
        raise SystemExit("Could not find 'Image component sizes' in the map file.") from exc

    entries: List[Dict[str, int]] = []
    for line in lines[start + 4 :]:
        stripped = line.strip()
        if not stripped:
            continue
        if stripped.startswith("-"):
            break
        tokens = line.split()
        if len(tokens) < 2:
            continue

        name = tokens[-1]
        if not name.endswith(".o"):
            continue

        number_tokens = tokens[:-1]
        if not all(token.lstrip("-").isdigit() for token in number_tokens):
            continue

        numbers = list(map(int, number_tokens))
        if len(numbers) == 6:
            code, ro, rw, extra, zi, debug = numbers
        elif len(numbers) == 5:
            code, ro, rw, zi, debug = numbers
            extra = 0
        else:  # pragma: no cover - unexpected format
            continue

        entries.append(
            {
                "object": name,
                "code": code,
                "ro": ro,
                "rw": rw,
                "zi": zi,
                "debug": debug,
                "extra": extra,
                "flash": code + ro + rw,
                "ram": rw + zi,
            }
        )

    if not entries:  # pragma: no cover - defensive
        raise SystemExit("No object entries were parsed from the map file.")

    return entries


def humanize_bytes(value: int) -> str:
    return f"{value / 1024:.2f} KB"


def build_table(
    entries: List[Dict[str, int]],
    sort_key: str,
    title: str,
    count: int,
    value_label: str,
) -> List[str]:
    sorted_entries = sorted(entries, key=lambda item: item[sort_key], reverse=True)
    if count <= 0 or count > len(sorted_entries):
        count = len(sorted_entries)
    lines = [
        title,
        f"{'Rank':>4}  {'Object':<35}  {'Code':>8}  {'RO':>8}  {'RW':>8}  {'ZI':>8}  {value_label:>12}  {'Human':>10}",
    ]

    for idx, entry in enumerate(sorted_entries[:count], 1):
        lines.append(
            f"{idx:>4}  {entry['object']:<35}  "
            f"{entry['code']:>8}  {entry['ro']:>8}  {entry['rw']:>8}  {entry['zi']:>8}  "
            f"{entry[sort_key]:>12}  {humanize_bytes(entry[sort_key]):>10}"
        )

    remaining = sum(entry[sort_key] for entry in sorted_entries[count:])
    remaining_objects = max(0, len(sorted_entries) - count)
    lines.append(
        f"...  remaining {remaining_objects} objects account for {remaining} bytes "
        f"({humanize_bytes(remaining)})"
    )
    lines.append("")
    return lines


def render_report(entries: List[Dict[str, int]], top_n: int) -> str:
    total_flash = sum(entry["flash"] for entry in entries)
    total_ram = sum(entry["ram"] for entry in entries)

    report_lines = [
        "RocketPi memory usage derived from rocketpi_factory.map",
        f"Objects parsed: {len(entries)}",
        f"Total Flash (Code + RO + RW): {total_flash} bytes ({humanize_bytes(total_flash)})",
        f"Total RAM (RW + ZI): {total_ram} bytes ({humanize_bytes(total_ram)})",
        "",
        "Top modules by Flash consumption:",
    ]
    report_lines += build_table(entries, "flash", "Flash focus", top_n, "Flash bytes")
    report_lines.append("Top modules by RAM consumption:")
    report_lines += build_table(entries, "ram", "RAM focus", top_n, "RAM bytes")
    return "\n".join(report_lines)


def main() -> None:
    parser = argparse.ArgumentParser(description="Analyze ARM map file memory usage.")
    parser.add_argument(
        "--map",
        default="rocketpi_factory.map",
        type=Path,
        help="Path to the .map file (default: rocketpi_factory.map)",
    )
    parser.add_argument(
        "--top",
        default=0,
        type=int,
        help="How many entries to display in each table (default: 0, meaning all entries)",
    )
    parser.add_argument(
        "--output",
        default="memory_usage_report.txt",
        type=Path,
        help="Where to write the report (default: memory_usage_report.txt)",
    )

    args = parser.parse_args()
    entries = parse_component_table(args.map)
    top_n = args.top if args.top > 0 else len(entries)
    report = render_report(entries, top_n)
    args.output.write_text(report, encoding="utf-8")
    print(f"Wrote {args.output} with {len(entries)} parsed object entries.")


if __name__ == "__main__":  # pragma: no cover - script entry
    main()
