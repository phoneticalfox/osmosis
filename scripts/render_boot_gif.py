#!/usr/bin/env python3
"""
Render a lightweight boot.gif from a captured boot log.

Usage:
    python3 scripts/render_boot_gif.py build/artifacts/boot.log build/artifacts/boot.gif

The script expects a plain-text log (e.g., produced by `make qemu | tee build/artifacts/boot.log`)
and emits a looping GIF that gradually reveals the log output using a simple terminal-style renderer.
"""
from __future__ import annotations

import sys
import re
from pathlib import Path
from typing import Iterable, List, Tuple

from PIL import Image, ImageDraw, ImageFont

ANSI_ESCAPE_RE = re.compile(r"\x1b[@-_][0-?]*[ -/]*[@-~]")
DEFAULT_COLUMNS = 96
DEFAULT_ROWS = 26
PADDING = 8
BG_COLOR = (6, 8, 12)
FG_COLOR = (0, 255, 120)
FRAME_DURATION_MS = 220
FINAL_HOLD_MS = 1400


def strip_ansi(line: str) -> str:
    return ANSI_ESCAPE_RE.sub("", line.rstrip("\n"))


def load_lines(log_path: Path) -> List[str]:
    text = log_path.read_text(errors="ignore").splitlines()
    return [strip_ansi(line) for line in text]


def ensure_dimensions(font: ImageFont.FreeTypeFont, columns: int, rows: int) -> Tuple[int, int]:
    sample = font.getbbox("M")
    char_width = sample[2] - sample[0]
    char_height = sample[3] - sample[1]
    width = char_width * columns + 2 * PADDING
    height = char_height * rows + 2 * PADDING
    return width, height


def render_frame(font: ImageFont.FreeTypeFont, lines: Iterable[str]) -> Image.Image:
    visible_lines = [line[:DEFAULT_COLUMNS] for line in list(lines)[-DEFAULT_ROWS:]]
    width, height = ensure_dimensions(font, DEFAULT_COLUMNS, DEFAULT_ROWS)
    image = Image.new("RGB", (width, height), color=BG_COLOR)
    draw = ImageDraw.Draw(image)

    sample = font.getbbox("M")
    char_height = sample[3] - sample[1]

    for row, line in enumerate(visible_lines):
        y = PADDING + row * char_height
        draw.text((PADDING, y), line, fill=FG_COLOR, font=font)

    return image


def build_frames(lines: List[str], font: ImageFont.FreeTypeFont) -> Tuple[List[Image.Image], List[int]]:
    frames: List[Image.Image] = []
    durations: List[int] = []
    buffer: List[str] = []

    for line in lines:
        buffer.append(line)
        frames.append(render_frame(font, buffer))
        durations.append(FRAME_DURATION_MS)

    if not frames:
        # fallback to a single empty frame
        frames.append(render_frame(font, []))
        durations.append(FINAL_HOLD_MS)
    else:
        # hold on the final frame a bit longer
        frames.append(frames[-1].copy())
        durations.append(FINAL_HOLD_MS)

    return frames, durations


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit("Usage: render_boot_gif.py <log path> <output gif>")

    log_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])

    if not log_path.exists():
        raise SystemExit(f"Log not found: {log_path}")

    output_path.parent.mkdir(parents=True, exist_ok=True)

    font = ImageFont.load_default()
    lines = load_lines(log_path)
    frames, durations = build_frames(lines, font)

    frames[0].save(
        output_path,
        save_all=True,
        append_images=frames[1:],
        loop=0,
        duration=durations,
        disposal=2,
    )
    print(f"Wrote {output_path} ({len(frames)} frames)")


if __name__ == "__main__":
    main()
