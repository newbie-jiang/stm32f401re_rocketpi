#!/usr/bin/env python3
"""
Batch convert every image under pic/ into raw RGB565 little-endian .bin frames.

The emitted files are sequentially named (000.bin, 001.bin, …) and placed
directly inside the output directory, which makes them SD-card friendly for
setups that only support 8.3 file names.
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp"}


def convert_one(src: Path, dst: Path, force: bool, ffmpeg: str) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)

    if dst.exists() and not force:
        return

    cmd = [
        ffmpeg,
        "-loglevel",
        "error",
        "-y",
        "-i",
        str(src),
        "-f",
        "rawvideo",
        "-pix_fmt",
        "rgb565le",
        str(dst),
    ]
    subprocess.run(cmd, check=True)


def run(root: Path, out_root: Path, ffmpeg: str, force: bool) -> list[Path]:
    sources = [
        entry
        for entry in sorted(root.rglob("*"))
        if entry.is_file() and entry.suffix.lower() in IMAGE_EXTENSIONS
    ]
    out_root.mkdir(parents=True, exist_ok=True)
    digits = max(3, len(str(max(len(sources) - 1, 0))))

    emitted: list[Path] = []
    for index, src in enumerate(sources):
        dst = out_root / f"{index:0{digits}d}.bin"
        convert_one(src, dst, force=force, ffmpeg=ffmpeg)
        emitted.append(dst)
    return emitted


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input",
        type=Path,
        default=Path("pic"),
        help="Root directory that stores the source images (default: %(default)s)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("pic_bin"),
        help="Output directory root for generated .bin files (default: %(default)s)",
    )
    parser.add_argument(
        "--ffmpeg",
        type=str,
        default="ffmpeg",
        help="ffmpeg executable name or path (default: %(default)s)",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing .bin files instead of skipping them",
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    try:
        emitted = run(args.input, args.output, args.ffmpeg, args.force)
    except FileNotFoundError as exc:  # pragma: no cover - user environment issue
        print(f"Missing file: {exc}", file=sys.stderr)
        return 2
    except subprocess.CalledProcessError as exc:  # pragma: no cover
        print(f"ffmpeg failed: {exc}", file=sys.stderr)
        return exc.returncode

    print(f"Generated {len(emitted)} frame binaries under {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
