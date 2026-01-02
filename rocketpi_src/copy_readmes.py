#!/usr/bin/env python3
"""
Copy every readme.md from project subdirectories into doc/ and
rename each copy to match the directory that contained the README.
"""

from __future__ import annotations

import shutil
from pathlib import Path


README_NAME = "readme.md"
MAX_DEPTH = 2


def iter_readmes(root: Path, doc_dir: Path):
    """Yield readme.md files from repo root and up to two directory levels."""

    def find_readme(directory: Path) -> Path | None:
        for entry in directory.iterdir():
            if entry.is_file() and entry.name.lower() == README_NAME:
                return entry
        return None

    def walk(directory: Path, depth: int):
        if directory == doc_dir or depth > MAX_DEPTH:
            return

        readme = find_readme(directory)
        if readme and doc_dir not in readme.parents:
            yield readme

        if depth == MAX_DEPTH:
            return

        for child in directory.iterdir():
            if child.is_dir():
                yield from walk(child, depth + 1)

    yield from walk(root, 0)


def main() -> None:
    repo_root = Path(__file__).resolve().parent
    doc_dir = repo_root / "doc"
    doc_dir.mkdir(parents=True, exist_ok=True)

    copied = []

    for readme in iter_readmes(repo_root, doc_dir):
        directory = readme.parent
        target_name = directory.name or repo_root.name
        relative_parent = directory.relative_to(repo_root)
        top_level = next((part for part in relative_parent.parts if part not in (".", "")), None)
        destination_dir = doc_dir / top_level if top_level else doc_dir
        destination_dir.mkdir(parents=True, exist_ok=True)
        destination = destination_dir / f"{target_name}.md"
        shutil.copy2(readme, destination)
        copied.append(
            (readme.relative_to(repo_root), destination.relative_to(repo_root))
        )

    if not copied:
        print("没有找到任何 readme.md 文件。")
        return

    print("已复制以下文件：")
    for src, dest in copied:
        print(f"{src} -> {dest}")


if __name__ == "__main__":
    main()
