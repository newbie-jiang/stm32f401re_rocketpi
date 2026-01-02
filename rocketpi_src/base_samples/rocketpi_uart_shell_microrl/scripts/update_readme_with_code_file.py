#!/usr/bin/env python3
"""
Append selected source files to the README as collapsible sections.
Selections can be configured through scripts/readme_sources.txt.
"""

from __future__ import annotations

import argparse
import hashlib
from dataclasses import dataclass
from pathlib import Path

LANGUAGE_MAP = {
    ".c": "c",
    ".h": "c",
}

ALLOWED_SUFFIXES = set(LANGUAGE_MAP)
START_MARKER = "<!-- BSP_DRIVERS_START -->"
END_MARKER = "<!-- BSP_DRIVERS_END -->"
DEFAULT_SOURCE_LIST = Path("scripts/readme_sources.txt")


@dataclass(frozen=True)
class FileEntry:
    path: Path
    relative_path: str
    language: str
    content: str


def detect_language(path: Path) -> str:
    return LANGUAGE_MAP.get(path.suffix.lower(), "")


def read_file(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace").rstrip()


def compute_digest(files: list[FileEntry]) -> str:
    digest = hashlib.sha256()
    for entry in files:
        digest.update(entry.relative_path.encode("utf-8"))
        digest.update(b"\0")
        digest.update(entry.content.encode("utf-8"))
        digest.update(b"\0")
    return digest.hexdigest()


def build_section(files: list[FileEntry], heading: str) -> str:
    file_digest = compute_digest(files)
    lines: list[str] = [
        START_MARKER,
        f"<!-- BSP_DRIVERS_HASH:{file_digest} -->",
        heading,
        "",
    ]

    for entry in files:
        lines.append("<details>")
        lines.append(f"<summary>{entry.relative_path}</summary>")
        lines.append("")
        fence = f"```{entry.language}" if entry.language else "```"
        lines.append(fence)
        lines.append(entry.content)
        lines.append("```")
        lines.append("")
        lines.append("</details>")
        lines.append("")

    lines.append(END_MARKER)

    return "\n".join(lines).rstrip() + "\n"


def replace_section(
    readme_content: str, heading: str, new_section: str
) -> tuple[str, bool]:
    new_section_clean = new_section.strip()
    start_idx = readme_content.find(START_MARKER)
    end_idx = readme_content.find(END_MARKER)

    if start_idx != -1 and end_idx != -1 and end_idx >= start_idx:
        end_idx += len(END_MARKER)
        existing_block = readme_content[start_idx:end_idx].strip()
        if existing_block == new_section_clean:
            return readme_content, True

        before = readme_content[:start_idx].rstrip()
        after = readme_content[end_idx:].lstrip()

        segments: list[str] = []
        if before:
            segments.append(before)
        segments.append(new_section_clean)
        if after:
            segments.append(after)

        updated = "\n\n".join(segments).rstrip() + "\n"
        return updated, False

    heading_idx = readme_content.find(heading)
    if heading_idx != -1:
        before = readme_content[:heading_idx].rstrip()
        updated = before + "\n\n" + new_section_clean + "\n"
        return updated, updated == readme_content

    trimmed = readme_content.rstrip()
    if trimmed:
        updated = trimmed + "\n\n" + new_section_clean + "\n"
    else:
        updated = new_section_clean + "\n"
    return updated, updated == readme_content


def make_file_entry(path: Path, repo_root: Path) -> FileEntry:
    try:
        relative_path = path.relative_to(repo_root).as_posix()
    except ValueError as exc:
        raise ValueError(f"Path {path} is outside the repository root") from exc
    if path.suffix.lower() not in ALLOWED_SUFFIXES:
        raise ValueError(f"Unsupported file type: {path}")
    return FileEntry(
        path=path,
        relative_path=relative_path,
        language=detect_language(path),
        content=read_file(path),
    )


def gather_directory_files(directory: Path, repo_root: Path) -> list[FileEntry]:
    entries: list[FileEntry] = []
    for path in sorted(
        (
            candidate
            for candidate in directory.rglob("*")
            if candidate.is_file() and candidate.suffix.lower() in ALLOWED_SUFFIXES
        ),
        key=lambda candidate: candidate.relative_to(directory),
    ):
        entries.append(make_file_entry(path, repo_root))
    return entries


def parse_source_config(config_path: Path) -> list[Path]:
    selections: list[Path] = []
    for line in config_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        selections.append(Path(stripped))
    return selections


def gather_configured_files(config_path: Path, repo_root: Path) -> list[FileEntry]:
    selections = parse_source_config(config_path)
    if not selections:
        raise RuntimeError(f"No entries defined in {config_path}")

    entries: list[FileEntry] = []
    for selection in selections:
        absolute_path = (repo_root / selection).resolve()
        if absolute_path.is_dir():
            entries.extend(gather_directory_files(absolute_path, repo_root))
        elif absolute_path.is_file():
            entries.append(make_file_entry(absolute_path, repo_root))
        else:
            raise FileNotFoundError(f"Configured path not found: {absolute_path}")

    if not entries:
        raise RuntimeError(
            f"No matching source files (.c/.h) found for selections in {config_path}"
        )
    return entries


def deduplicate_and_sort_files(files: list[FileEntry]) -> list[FileEntry]:
    unique: dict[str, FileEntry] = {}
    for entry in files:
        unique[entry.relative_path] = entry
    return [unique[key] for key in sorted(unique)]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Append configured source files to README.md."
    )
    parser.add_argument(
        "--sources-config",
        default=None,
        help=(
            "Path to file listing files/directories to include "
            f"(default: {DEFAULT_SOURCE_LIST})"
        ),
    )
    parser.add_argument(
        "--readme",
        default="readme.md",
        help="Path to README file (default: %(default)s)",
    )
    parser.add_argument(
        "--heading",
        default="## 驱动以及测试代码",
        help="Heading inserted before the file list (default: %(default)s)",
    )
    args = parser.parse_args()
    args.sources_config_provided = args.sources_config is not None
    if args.sources_config is None:
        args.sources_config = str(DEFAULT_SOURCE_LIST)
    return args


def main() -> None:
    args = parse_args()
    repo_root = Path().resolve()
    config_path = (repo_root / args.sources_config).resolve()
    readme_path = (repo_root / args.readme).resolve()

    if not readme_path.is_file():
        raise FileNotFoundError(f"README file not found: {readme_path}")

    if not config_path.is_file():
        raise FileNotFoundError(f"Configured source list not found: {config_path}")

    files = deduplicate_and_sort_files(
        gather_configured_files(config_path, repo_root)
    )

    new_section = build_section(files, args.heading)
    original_readme = readme_path.read_text(encoding="utf-8")
    updated_readme, unchanged = replace_section(
        original_readme, args.heading, new_section
    )

    if unchanged:
        print("README already up to date. No changes written.")
        return

    readme_path.write_text(updated_readme, encoding="utf-8")
    print("README updated with BSP driver sources.")


if __name__ == "__main__":
    main()
