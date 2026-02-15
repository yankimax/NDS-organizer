#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def quantize_to_16_grays(img_l: Image.Image) -> Image.Image:
    img_p = img_l.point(lambda p: p // 17).convert("P")
    palette = []
    for i in range(16):
        v = i * 17
        palette.extend([v, v, v])
    palette.extend([0, 0, 0] * (256 - 16))
    img_p.putpalette(palette)
    return img_p


def safe_decode_cp1251(code: int) -> str:
    try:
        return bytes([code]).decode("cp1251")
    except UnicodeDecodeError:
        return " "


def draw_centered_text(
    draw: ImageDraw.ImageDraw,
    x0: int,
    y0: int,
    w: int,
    h: int,
    text: str,
    font: ImageFont.FreeTypeFont,
    fill: int,
) -> None:
    bbox = draw.textbbox((0, 0), text, font=font)
    if bbox is None:
        return
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    x = x0 + (w - tw) // 2 - bbox[0]
    y = y0 + (h - th) // 2 - bbox[1]
    draw.text((x, y), text, fill=fill, font=font)


def generate_console_font(args: argparse.Namespace) -> None:
    input_ttf = Path(args.input)
    output = Path(args.output)
    if not input_ttf.exists():
        raise FileNotFoundError(f"Input font not found: {input_ttf}")

    glyph_count = args.end - args.start + 1
    if glyph_count <= 0:
        raise ValueError("--end must be >= --start")

    scale = max(1, args.oversample)
    w = args.cell_width
    h = args.cell_height

    img = Image.new("L", (w * scale, glyph_count * h * scale), 0)
    draw = ImageDraw.Draw(img)
    font = ImageFont.truetype(str(input_ttf), size=args.font_size * scale)

    for code in range(args.start, args.end + 1):
        ch = safe_decode_cp1251(code) if args.encoding == "cp1251" else chr(code)
        y = (code - args.start) * h * scale
        draw_centered_text(draw, 0, y, w * scale, h * scale, ch, font, 255)

    if scale > 1:
        img = img.resize((w, glyph_count * h), Image.Resampling.LANCZOS)

    out = quantize_to_16_grays(img)
    ensure_parent(output)
    out.save(output)
    print(f"Generated {output} ({out.width}x{out.height})")


def draw_seven_segment(draw: ImageDraw.ImageDraw, x0: int, y0: int, w: int, h: int, digit: int, fill: tuple[int, int, int]) -> None:
    seg_map = [0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F]
    seg = seg_map[digit]
    t = max(1, (h // 8) - 1)
    m = max(1, h // 10)
    mid = y0 + h // 2

    def rect(x: int, y: int, rw: int, rh: int) -> None:
        draw.rectangle([x, y, x + rw - 1, y + rh - 1], fill=fill)

    if seg & (1 << 0):
        rect(x0 + m + t, y0 + m, w - 2 * (m + t), t)
    if seg & (1 << 1):
        rect(x0 + w - m - t, y0 + m + t, t, h // 2 - (m + t) - 1)
    if seg & (1 << 2):
        rect(x0 + w - m - t, mid + 1, t, h // 2 - m - t - 1)
    if seg & (1 << 3):
        rect(x0 + m + t, y0 + h - m - t, w - 2 * (m + t), t)
    if seg & (1 << 4):
        rect(x0 + m, mid + 1, t, h // 2 - m - t - 1)
    if seg & (1 << 5):
        rect(x0 + m, y0 + m + t, t, h // 2 - (m + t) - 1)
    if seg & (1 << 6):
        rect(x0 + m + t, mid - (t // 2), w - 2 * (m + t), t)


def generate_digits_font(args: argparse.Namespace) -> None:
    output = Path(args.output)
    glyphs = args.glyphs
    if not glyphs:
        raise ValueError("--glyphs cannot be empty")

    bg = (255, 0, 255)
    fg = (255, 255, 255)
    cell_w = args.cell_width
    cell_h = args.cell_height

    img = Image.new("RGB", (cell_w * len(glyphs), cell_h), bg)
    draw = ImageDraw.Draw(img)

    if args.style == "sevenseg":
        for i, ch in enumerate(glyphs):
            x0 = i * cell_w
            if ch.isdigit():
                draw_seven_segment(draw, x0, 0, cell_w, cell_h, int(ch), fg)
            elif ch == ":":
                dot = max(2, cell_h // 8)
                cx = x0 + (cell_w - dot) // 2
                y1 = cell_h // 3 - dot // 2
                y2 = (cell_h * 2) // 3 - dot // 2
                draw.rectangle([cx, y1, cx + dot - 1, y1 + dot - 1], fill=fg)
                draw.rectangle([cx, y2, cx + dot - 1, y2 + dot - 1], fill=fg)
    else:
        input_ttf = Path(args.input)
        if not input_ttf.exists():
            raise FileNotFoundError(f"Input font not found: {input_ttf}")
        scale = max(1, args.oversample)
        hi = Image.new("RGB", (cell_w * len(glyphs) * scale, cell_h * scale), bg)
        hi_draw = ImageDraw.Draw(hi)
        font = ImageFont.truetype(str(input_ttf), size=args.font_size * scale)
        for i, ch in enumerate(glyphs):
            draw_centered_text(
                hi_draw,
                i * cell_w * scale,
                0,
                cell_w * scale,
                cell_h * scale,
                ch,
                font,
                fill=255,
            )
        img = hi.resize((cell_w * len(glyphs), cell_h), Image.Resampling.LANCZOS)

    ensure_parent(output)
    img.save(output)
    print(f"Generated {output} ({img.width}x{img.height})")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Generate NDS-friendly bitmap fonts from TTF.")
    sub = parser.add_subparsers(dest="mode", required=True)

    p_console = sub.add_parser("console", help="Generate vertical glyph atlas for consoleSetFont.")
    p_console.add_argument("--input", required=True, help="Path to .ttf file")
    p_console.add_argument("--output", required=True, help="Output image path (usually .png)")
    p_console.add_argument("--encoding", default="cp1251", choices=["cp1251", "latin1"])
    p_console.add_argument("--start", type=int, default=32)
    p_console.add_argument("--end", type=int, default=255)
    p_console.add_argument("--cell-width", type=int, default=8)
    p_console.add_argument("--cell-height", type=int, default=8)
    p_console.add_argument("--font-size", type=int, default=9)
    p_console.add_argument("--oversample", type=int, default=1)
    p_console.set_defaults(func=generate_console_font)

    p_digits = sub.add_parser("digits", help="Generate horizontal atlas for big time digits.")
    p_digits.add_argument("--input", default="", help="Path to .ttf (required when --style=ttf)")
    p_digits.add_argument("--output", required=True, help="Output image path (usually .png)")
    p_digits.add_argument("--glyphs", default="0123456789:")
    p_digits.add_argument("--cell-width", type=int, default=32)
    p_digits.add_argument("--cell-height", type=int, default=32)
    p_digits.add_argument("--style", choices=["sevenseg", "ttf"], default="sevenseg")
    p_digits.add_argument("--font-size", type=int, default=30)
    p_digits.add_argument("--oversample", type=int, default=2)
    p_digits.set_defaults(func=generate_digits_font)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    args.func(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
