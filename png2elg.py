#https://github.com/Xx-vicen-xX/Lucifen-Tools/blob/main/png2elg.py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse, os, struct
from typing import Tuple
from PIL import Image  # pip install pillow

# Signatures used by the decoder (little-endian)
ELG_SIGS = {
    8:  0x08474C45,  # "ELG\x08"
    24: 0x18474C45,  # "ELG\x18"
    32: 0x20474C45,  # "ELG\x20"
}

def _pack_u16(v: int) -> bytes:
    return struct.pack('<H', v & 0xFFFF)

def _encode_literals_bytes(data: bytes) -> bytes:
    """
    Encode arbitrary byte stream using only 'literal' packets for the
    ELG 8-bit palette/indices stream (and alpha stream). Packet format:

      flags (top two bits 00, bit 5=0) => count = (flags & 0x1F) + 1,  max 32
      then 'count' literal bytes
    """
    out = bytearray()
    i, n = 0, len(data)
    while i < n:
        count = min(32, n - i)
        flags = (count - 1) & 0x1F  # 00abcde with range 0..31 => 1..32 bytes
        out.append(flags)
        out.extend(data[i:i+count])
        i += count
    out.append(0xFF)  # end marker
    return bytes(out)

def _encode_literals_pixels_rgb(bgr_triplets: bytes) -> bytes:
    """
    Encode RGB/BGR pixel stream using only 'literal' packets.
    For ELG 24-bit and the color pass of 32-bit, the decoder interprets
    literal 'count' as *pixels* (3 bytes per pixel).
    """
    out = bytearray()
    n_pix = len(bgr_triplets) // 3
    i_pix = 0
    while i_pix < n_pix:
        count = min(32, n_pix - i_pix)  # pixels
        flags = (count - 1) & 0x1F
        out.append(flags)
        start = i_pix * 3
        out.extend(bgr_triplets[start:start + 3*count])
        i_pix += count
    out.append(0xFF)
    return bytes(out)

def _split_rgba_to_bgr_and_a(rgba: bytes) -> Tuple[bytes, bytes]:
    """Return (BGR stream for color pass, Alpha bytes for alpha pass)."""
    n = len(rgba) // 4
    bgr = bytearray(3*n)
    a   = bytearray(n)
    j = 0
    for i in range(n):
        r = rgba[j]; g = rgba[j+1]; b = rgba[j+2]; a[i] = rgba[j+3]
        # ELG color stream uses B, G, R (alpha set to 0xFF during decode color pass)
        bgr[3*i+0] = b
        bgr[3*i+1] = g
        bgr[3*i+2] = r
        j += 4
    return bytes(bgr), bytes(a)

def _rgb_to_bgr(rgb: bytes) -> bytes:
    n = len(rgb) // 3
    bgr = bytearray(len(rgb))
    j = 0
    for i in range(n):
        r = rgb[j]; g = rgb[j+1]; b = rgb[j+2]
        bgr[3*i+0] = b
        bgr[3*i+1] = g
        bgr[3*i+2] = r
        j += 3
    return bytes(bgr)

def _ensure_palette_256(im: Image.Image) -> bytes:
    """
    Build a 256-entry BGRA palette (1024 bytes).
    The ELG decoder expects a 0x400-byte palette unpack first (BGRA each).
    """
    # Pillow palette is RGB triplets length 768 (256*3) typically.
    pal = im.getpalette()
    if pal is None:
        # No palette? Build grayscale as fallback.
        pal = []
        for i in range(256):
            pal.extend((i, i, i))
    # Ensure length 256*3
    if len(pal) < 768:
        pal = pal + [0] * (768 - len(pal))
    else:
        pal = pal[:768]
    # Convert to BGRA entries (alpha 0xFF)
    out = bytearray(1024)
    for i in range(256):
        r = pal[3*i + 0]
        g = pal[3*i + 1]
        b = pal[3*i + 2]
        out[4*i + 0] = b
        out[4*i + 1] = g
        out[4*i + 2] = r
        out[4*i + 3] = 0xFF
    return bytes(out)

def encode_elg_from_image(im: Image.Image) -> bytes:
    """
    Create a minimal ELG (type-0) for modes: P (8bpp), RGB (24bpp), RGBA (32bpp).
    Streams are literal-only; valid for the reference decoder.
    Header layout (type-0):
        [ 'ELG' + bpp_byte ][ width:u16 ][ height:u16 ]
    """
    # Normalize mode
    mode = im.mode
    if mode == 'P':
        bpp = 8
        w, h = im.size
        sig = ELG_SIGS[bpp]
        header = struct.pack('<I', sig) + _pack_u16(w) + _pack_u16(h)
        pal_bgra = _ensure_palette_256(im)
        pix = im.tobytes()  # index bytes length w*h
        body = bytearray()
        body += _encode_literals_bytes(pal_bgra)   # palette stream
        body += _encode_literals_bytes(pix)        # indices stream
        return bytes(header + body)

    # Handle grayscale by quantizing to palette (keeps file 8bpp)
    if mode in ('L',):
        im = im.convert('P')
        return encode_elg_from_image(im)

    if mode in ('LA',):
        im = im.convert('RGBA')
        mode = 'RGBA'

    if mode not in ('RGB','RGBA'):
        im = im.convert('RGBA' if 'A' in mode else 'RGB')
        mode = im.mode

    w, h = im.size
    if mode == 'RGB':
        bpp = 24
        sig = ELG_SIGS[bpp]
        header = struct.pack('<I', sig) + _pack_u16(w) + _pack_u16(h)
        rgb = im.tobytes()
        bgr = _rgb_to_bgr(rgb)
        body = _encode_literals_pixels_rgb(bgr)
        return bytes(header + body)

    # RGBA
    bpp = 32
    sig = ELG_SIGS[bpp]
    header = struct.pack('<I', sig) + _pack_u16(w) + _pack_u16(h)
    rgba = im.tobytes()
    bgr, alpha = _split_rgba_to_bgr_and_a(rgba)
    color_stream = _encode_literals_pixels_rgb(bgr)  # decoder sets A=0xFF during this pass
    alpha_stream = _encode_literals_bytes(alpha)     # then alpha stream overwrites A
    return bytes(header + color_stream + alpha_stream)

def convert_one(src_png: str, dst_elg: str):
    im = Image.open(src_png)
    data = encode_elg_from_image(im)
    os.makedirs(os.path.dirname(dst_elg) or ".", exist_ok=True)
    with open(dst_elg, "wb") as f:
        f.write(data)

def batch_convert(src_path: str, dst_path: str):
    if os.path.isfile(src_path):
        if os.path.isdir(dst_path):
            base = os.path.splitext(os.path.basename(src_path))[0] + ".elg"
            convert_one(src_path, os.path.join(dst_path, base))
        else:
            convert_one(src_path, dst_path)
    else:
        os.makedirs(dst_path, exist_ok=True)
        for root, _, files in os.walk(src_path):
            for fn in files:
                if fn.lower().endswith(".png"):
                    in_fp = os.path.join(root, fn)
                    rel = os.path.relpath(in_fp, src_path)
                    out_fp = os.path.join(dst_path, os.path.splitext(rel)[0] + ".elg")
                    os.makedirs(os.path.dirname(out_fp), exist_ok=True)
                    convert_one(in_fp, out_fp)

def main():
    ap = argparse.ArgumentParser(description="Convert PNG → ELG (type-0, literal-only packets).")
    ap.add_argument("input", help="PNG file or a folder of PNGs")
    ap.add_argument("output", help="Output .elg path (single) or a folder (batch)")
    args = ap.parse_args()
    batch_convert(args.input, args.output)
    print("Done.")

if __name__ == "__main__":
    main()
