#!/usr/bin/env python3
"""
Discover and run every scripts/update_readme_with_code_file.py that exists under
this repository. Each script is executed from its project root so relative
paths such as scripts/readme_sources.txt keep working.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

SCRIPT_RELATIVE_PATH = Path("scripts") / "update_readme_with_code_file.py"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run every scripts/update_readme_with_code_file.py found in the tree."
        )
    )
    parser.add_argument(
        "--root",
        default=None,
        help=(
            "Root directory to search (default: directory containing this script)."
        ),
    )
    parser.add_argument(
        "--fail-fast",
        action="store_true",
        help="Stop on the first failure instead of running all scripts.",
    )
    return parser.parse_args()


def discover_scripts(root: Path) -> list[Path]:
    candidates = sorted(
        root.rglob(str(SCRIPT_RELATIVE_PATH)),
        key=lambda path: path.relative_to(root),
    )
    return candidates


def find_readme(project_root: Path) -> Path | None:
    """Locate README file ignoring case (prefer exact match)."""
    preferred = project_root / "readme.md"
    if preferred.is_file():
        return preferred

    for entry in project_root.iterdir():
        if entry.is_file() and entry.name.lower() == "readme.md":
            return entry
    return None


def run_script(target: Path) -> int:
    project_root = target.parent.parent
    rel_target = target.relative_to(project_root)
    print(f"Running {project_root.name}/{rel_target.as_posix()} ...")
    cmd = [sys.executable, str(target)]
    readme_path = find_readme(project_root)
    if readme_path:
        cmd.extend(["--readme", readme_path.name])
    completed = subprocess.run(cmd, cwd=project_root)
    return completed.returncode


def main() -> int:
    args = parse_args()
    root = Path(args.root).resolve() if args.root else Path(__file__).resolve().parent

    scripts = discover_scripts(root)
    if not scripts:
        print("No update_readme_with_code_file.py scripts found.", file=sys.stderr)
        return 1

    failures: list[tuple[Path, int]] = []
    for index, target in enumerate(scripts, start=1):
        rel = target.relative_to(root)
        print(f"[{index}/{len(scripts)}] {rel.as_posix()}")
        code = run_script(target)
        if code != 0:
            failures.append((target, code))
            print(f"    -> Failed with exit code {code}", file=sys.stderr)
            if args.fail_fast:
                break

    if failures:
        print("\nSummary of failures:", file=sys.stderr)
        for target, code in failures:
            print(
                f"  {target.relative_to(root).as_posix()} (exit code {code})",
                file=sys.stderr,
            )
        return 1

    print("All update scripts completed successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
