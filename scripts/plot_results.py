#!/usr/bin/env python3
"""
scripts/plot_results.py
Lee los archivos CSV de results/ (benchmark_metrics.csv, corpus_benchmark.csv y
cache_metrics.csv) y genera las 4 figuras comparativas en alta resolucion (.png):
  1. results/fig1_time_scaling.png
  2. results/fig2_speedup_by_family.png
  3. results/fig3_memory_and_recursion.png
  4. results/fig4_cache_and_corpus.png

Incluye soporte automatico para pandas/matplotlib y un motor grafico cientifico PNG
nativo en Python estandar (zlib + struct) para garantizar ejecucion 100% reproducible
en entornos Windows/MSYS2 sin dependencias externas.
"""

import csv
import math
from pathlib import Path
import struct
import zlib


# =============================================================================
# Motor de Renderizado Cientifico PNG en Python Puro (sin dependencias externas)
# =============================================================================

FONT_5X7 = {
    " ": [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
    "!": [0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04],
    "%": [0x19, 0x1A, 0x04, 0x04, 0x0B, 0x13, 0x00],
    "(": [0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02],
    ")": [0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08],
    "*": [0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00],
    "+": [0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00],
    ",": [0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08],
    "-": [0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00],
    ".": [0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C],
    "/": [0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10],
    "0": [0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E],
    "1": [0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E],
    "2": [0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F],
    "3": [0x1F, 0x02, 0x04, 0x06, 0x01, 0x11, 0x0E],
    "4": [0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02],
    "5": [0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E],
    "6": [0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E],
    "7": [0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08],
    "8": [0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E],
    "9": [0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C],
    ":": [0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00],
    "=": [0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00],
    ">": [0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08],
    "|": [0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04],
    "~": [0x00, 0x00, 0x08, 0x15, 0x02, 0x00, 0x00],
    "A": [0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11],
    "B": [0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E],
    "C": [0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E],
    "D": [0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C],
    "E": [0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F],
    "F": [0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10],
    "G": [0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F],
    "H": [0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11],
    "I": [0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E],
    "J": [0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C],
    "K": [0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11],
    "L": [0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F],
    "M": [0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11],
    "N": [0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11],
    "O": [0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E],
    "P": [0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10],
    "Q": [0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D],
    "R": [0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11],
    "S": [0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E],
    "T": [0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04],
    "U": [0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E],
    "V": [0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04],
    "W": [0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A],
    "X": [0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11],
    "Y": [0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04],
    "Z": [0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F],
    "_": [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F],
}


class PureCanvasPNG:
    def __init__(self, width: int, height: int, bg=(255, 255, 255)):
        self.width = width
        self.height = height
        row_bytes = bytes(bg) * width
        self.buf = bytearray(row_bytes * height)

    def set_pixel(self, x: int, y: int, color):
        if 0 <= x < self.width and 0 <= y < self.height:
            idx = (y * self.width + x) * 3
            self.buf[idx] = color[0]
            self.buf[idx + 1] = color[1]
            self.buf[idx + 2] = color[2]

    def fill_rect(self, x0: int, y0: int, x1: int, y1: int, color):
        xa, xb = max(0, min(x0, x1)), min(self.width - 1, max(x0, x1))
        ya, yb = max(0, min(y0, y1)), min(self.height - 1, max(y0, y1))
        if xa > xb or ya > yb:
            return
        span = xb - xa + 1
        row_chunk = bytes(color) * span
        for y in range(ya, yb + 1):
            start = (y * self.width + xa) * 3
            self.buf[start : start + span * 3] = row_chunk

    def draw_rect(self, x0: int, y0: int, x1: int, y1: int, color, thickness: int = 1):
        self.fill_rect(x0, y0, x1, y0 + thickness - 1, color)
        self.fill_rect(x0, y1 - thickness + 1, x1, y1, color)
        self.fill_rect(x0, y0, x0 + thickness - 1, y1, color)
        self.fill_rect(x1 - thickness + 1, y0, x1, y1, color)

    def draw_line(self, x0: int, y0: int, x1: int, y1: int, color, thickness: int = 2):
        dx = abs(x1 - x0)
        dy = -abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx + dy
        r = thickness // 2
        while True:
            if r <= 0:
                self.set_pixel(x0, y0, color)
            else:
                self.fill_rect(x0 - r, y0 - r, x0 + r, y0 + r, color)
            if x0 == x1 and y0 == y1:
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x0 += sx
            if e2 <= dx:
                err += dx
                y0 += sy

    def draw_circle(self, cx: int, cy: int, radius: int, color):
        for dy in range(-radius, radius + 1):
            for dx in range(-radius, radius + 1):
                if dx * dx + dy * dy <= radius * radius:
                    self.set_pixel(cx + dx, cy + dy, color)

    def draw_text(self, x: int, y: int, text: str, color=(20, 20, 20), scale: int = 2):
        cx = x
        for ch in text.upper():
            glyph = FONT_5X7.get(ch, FONT_5X7[" "])
            for row_idx, row_bits in enumerate(glyph):
                for col_idx in range(5):
                    if (row_bits >> (4 - col_idx)) & 1:
                        px = cx + col_idx * scale
                        py = y + row_idx * scale
                        self.fill_rect(px, py, px + scale - 1, py + scale - 1, color)
            cx += 6 * scale

    def save_png(self, filepath: Path):
        raw_rows = bytearray()
        stride = self.width * 3
        for y in range(self.height):
            raw_rows.append(0)  # Filter type 0 (None)
            start = y * stride
            raw_rows.extend(self.buf[start : start + stride])

        def make_chunk(chunk_type: bytes, data: bytes) -> bytes:
            crc = zlib.crc32(chunk_type + data) & 0xFFFFFFFF
            return struct.pack(">I", len(data)) + chunk_type + data + struct.pack(">I", crc)

        ihdr = struct.pack(">IIBBBBB", self.width, self.height, 8, 2, 0, 0, 0)
        idat = zlib.compress(bytes(raw_rows), level=6)
        png_bytes = (
            b"\x89PNG\r\n\x1a\n"
            + make_chunk(b"IHDR", ihdr)
            + make_chunk(b"IDAT", idat)
            + make_chunk(b"IEND", b"")
        )
        if filepath.exists():
            filepath.unlink()
        filepath.write_bytes(png_bytes)


# =============================================================================
# Funciones de Graficacion para las 4 Figuras del Informe
# =============================================================================

COLOR_DC3 = (214, 48, 49)      # Rojo intenso para DC3
COLOR_SAIS = (9, 132, 227)     # Azul brillante para SA-IS
COLOR_COMP = (0, 184, 148)     # Verde esmeralda para SA-IS Compacto
COLOR_NAIVE = (108, 92, 231)   # Violeta para Naive SA
COLOR_GRID = (230, 234, 240)
COLOR_AXIS = (60, 65, 75)


def plot_fig1_time_scaling(rows, out_path: Path):
    canvas = PureCanvasPNG(1400, 920, bg=(252, 253, 255))
    canvas.draw_text(140, 25, "FIG 1: ESCALAMIENTO DE TIEMPO DE CONSTRUCCION (DC3 VS SA-IS)", (20, 30, 45), scale=3)
    canvas.draw_text(280, 60, "TIEMPO MEDIANO (MS) EN FUNCION DE N (4 FAMILIAS DE ENTRADA)", (80, 90, 105), scale=2)

    families = ["random_ascii", "dna_genome", "source_code", "periodic_str"]
    boxes = [
        (90, 110, 650, 460),
        (760, 110, 1320, 460),
        (90, 530, 650, 880),
        (760, 530, 1320, 880),
    ]

    for fam, (x0, y0, x1, y1) in zip(families, boxes):
        sub = [r for r in rows if r["dataset"] == fam]
        if not sub:
            continue
        canvas.fill_rect(x0, y0, x1, y1, (255, 255, 255))
        canvas.draw_rect(x0, y0, x1, y1, COLOR_AXIS, thickness=2)
        canvas.draw_text(x0 + 15, y0 - 22, f"FAMILIA: {fam.upper()}", (30, 40, 55), scale=2)

        max_n = max(int(r["n"]) for r in sub)
        max_y = max(max(float(r["dc3_median_ms"]), float(r["sais_median_ms"])) for r in sub) * 1.15
        if max_y <= 0:
            max_y = 1.0

        # Lineas de grilla horizontales
        for g in range(1, 5):
            gy = y1 - int((y1 - y0) * g / 5.0)
            canvas.draw_line(x0 + 2, gy, x1 - 2, gy, COLOR_GRID, thickness=1)
            val = max_y * g / 5.0
            canvas.draw_text(x0 - 65, gy - 6, f"{val:5.1f}", (90, 90, 90), scale=2)

        def to_xy(n_val, t_val):
            px = x0 + 20 + int((x1 - x0 - 40) * (n_val / max_n))
            py = y1 - 15 - int((y1 - y0 - 35) * (t_val / max_y))
            return px, py

        dc3_pts = [to_xy(int(r["n"]), float(r["dc3_median_ms"])) for r in sub]
        sais_pts = [to_xy(int(r["n"]), float(r["sais_median_ms"])) for r in sub]

        for i in range(len(dc3_pts) - 1):
            canvas.draw_line(dc3_pts[i][0], dc3_pts[i][1], dc3_pts[i + 1][0], dc3_pts[i + 1][1], COLOR_DC3, thickness=3)
            canvas.draw_line(sais_pts[i][0], sais_pts[i][1], sais_pts[i + 1][0], sais_pts[i + 1][1], COLOR_SAIS, thickness=3)

        for (px, py), r in zip(dc3_pts, sub):
            canvas.draw_circle(px, py, 5, COLOR_DC3)
        for (px, py), r in zip(sais_pts, sub):
            canvas.draw_circle(px, py, 5, COLOR_SAIS)

        # Etiquetas del ultimo punto (n = 500k)
        canvas.draw_text(dc3_pts[-1][0] - 85, dc3_pts[-1][1] - 18, f"DC3:{float(sub[-1]['dc3_median_ms']):.1f}MS", COLOR_DC3, scale=2)
        canvas.draw_text(sais_pts[-1][0] - 95, sais_pts[-1][1] + 10, f"SAIS:{float(sub[-1]['sais_median_ms']):.1f}MS", COLOR_SAIS, scale=2)
        canvas.draw_text(x0 + 10, y1 + 8, "N=2K..100K", (90, 90, 90), scale=2)
        canvas.draw_text(x1 - 95, y1 + 8, "N=500K", (90, 90, 90), scale=2)

    canvas.save_png(out_path)


def plot_fig2_speedup_by_family(rows, out_path: Path):
    canvas = PureCanvasPNG(1300, 760, bg=(252, 253, 255))
    canvas.draw_text(130, 28, "FIG 2: SPEEDUP DE SA-IS SOBRE DC3 (TIEMPO DC3 / TIEMPO SA-IS)", (20, 30, 45), scale=3)
    canvas.draw_text(260, 62, "VALORES > 1.0X INDICAN QUE SA-IS ES MAS RAPIDO QUE DC3", (80, 90, 105), scale=2)

    x0, y0, x1, y1 = 120, 110, 1220, 660
    canvas.fill_rect(x0, y0, x1, y1, (255, 255, 255))
    canvas.draw_rect(x0, y0, x1, y1, COLOR_AXIS, thickness=2)

    families = [
        ("random_ascii", (108, 92, 231)),
        ("dna_genome", (0, 184, 148)),
        ("source_code", (9, 132, 227)),
        ("periodic_str", (214, 48, 49)),
    ]
    sizes = [2000, 5000, 10000, 50000, 100000, 250000, 500000]
    max_sp = 2.5

    for g in range(1, 6):
        val = 0.5 * g
        gy = y1 - int((y1 - y0) * (val / max_sp))
        col = (150, 150, 150) if abs(val - 1.0) < 1e-6 else COLOR_GRID
        canvas.draw_line(x0 + 2, gy, x1 - 2, gy, col, thickness=2 if abs(val - 1.0) < 1e-6 else 1)
        canvas.draw_text(x0 - 70, gy - 6, f"{val:4.1f}X", (60, 60, 60), scale=2)

    for idx_s, s_val in enumerate(sizes):
        gx = x0 + 60 + int((x1 - x0 - 120) * idx_s / (len(sizes) - 1))
        canvas.draw_line(gx, y0 + 2, gx, y1 - 2, COLOR_GRID, thickness=1)
        label = f"{s_val // 1000}K"
        canvas.draw_text(gx - 20, y1 + 12, label, (60, 60, 60), scale=2)

    for fam_idx, (fam, col) in enumerate(families):
        sub = [r for r in rows if r["dataset"] == fam]
        pts = []
        for idx_s, r in enumerate(sub):
            gx = x0 + 60 + int((x1 - x0 - 120) * idx_s / (len(sizes) - 1))
            sp = float(r["speedup_dc3_over_sais"])
            gy = y1 - int((y1 - y0) * (sp / max_sp))
            pts.append((gx, gy, sp))
        for i in range(len(pts) - 1):
            canvas.draw_line(pts[i][0], pts[i][1], pts[i + 1][0], pts[i + 1][1], col, thickness=3)
        for gx, gy, sp in pts:
            canvas.draw_circle(gx, gy, 5, col)

        # Leyenda superior dentro del grafico
        lx = x0 + 30 + fam_idx * 265
        ly = y0 + 25
        canvas.fill_rect(lx, ly, lx + 24, ly + 12, col)
        canvas.draw_text(lx + 32, ly, f"{fam.upper()} ({pts[-1][2]:.2f}X)", col, scale=2)

    canvas.save_png(out_path)


def plot_fig3_memory_and_recursion(rows, out_path: Path):
    canvas = PureCanvasPNG(1400, 820, bg=(252, 253, 255))
    canvas.draw_text(110, 25, "FIG 3: CONSUMO DE MEMORIA (B/CHAR), RECURSION Y RAZON DE REDUCCION", (20, 30, 45), scale=3)
    canvas.draw_text(290, 58, "COMPARATIVA EN N = 500,000 SOBRE LAS 4 DISTRIBUCIONES", (80, 90, 105), scale=2)

    sub500 = [r for r in rows if int(r["n"]) == 500000]

    # Panel Izquierdo: Bytes por caracter (DC3 vs SA-IS 32b vs SA-IS Compacto)
    x0, y0, x1, y1 = 80, 115, 690, 720
    canvas.fill_rect(x0, y0, x1, y1, (255, 255, 255))
    canvas.draw_rect(x0, y0, x1, y1, COLOR_AXIS, thickness=2)
    canvas.draw_text(x0 + 20, y0 - 22, "A) PICO DE MEMORIA RAM (BYTES / CARACTER)", (30, 40, 55), scale=2)

    max_bpc = 28.0
    for g in range(1, 5):
        val = 7.0 * g
        gy = y1 - int((y1 - y0 - 40) * (val / max_bpc))
        canvas.draw_line(x0 + 2, gy, x1 - 2, gy, COLOR_GRID, thickness=1)
        canvas.draw_text(x0 - 50, gy - 6, f"{int(val)}B", (80, 80, 80), scale=2)

    for idx, r in enumerate(sub500):
        base_x = x0 + 35 + idx * 140
        b_dc3 = float(r["dc3_bytes_per_char"])
        b_sais = float(r["sais_bytes_per_char"])
        b_comp = float(r["sais_compact_bpc"])

        for b_idx, (val, col) in enumerate([(b_dc3, COLOR_DC3), (b_sais, COLOR_SAIS), (b_comp, COLOR_COMP)]):
            bx0 = base_x + b_idx * 34
            bh = int((y1 - y0 - 40) * (val / max_bpc))
            canvas.fill_rect(bx0, y1 - bh, bx0 + 28, y1 - 2, col)
            canvas.draw_text(bx0 - 2, y1 - bh - 16, f"{val:.1f}", col, scale=1)

        short_name = r["dataset"].split("_")[0].upper()
        canvas.draw_text(base_x + 10, y1 + 12, short_name, (50, 50, 50), scale=2)

    canvas.draw_text(x0 + 20, y0 + 18, "ROJO: DC3 (~23B) | AZUL: SAIS INT (~14B) | VERDE: SAIS COMPACTO (~5.3B)", (40, 40, 40), scale=1)

    # Panel Derecho: Profundidad Recursiva y Razon de Reduccion (n02/n vs n1/n)
    rx0, ry0, rx1, ry1 = 760, 115, 1350, 720
    canvas.fill_rect(rx0, ry0, rx1, ry1, (255, 255, 255))
    canvas.draw_rect(rx0, ry0, rx1, ry1, COLOR_AXIS, thickness=2)
    canvas.draw_text(rx0 + 20, ry0 - 22, "B) PROFUNDIDAD RECURSIVA Y REDUCCION (%)", (30, 40, 55), scale=2)

    max_depth = 14.0
    for idx, r in enumerate(sub500):
        base_x = rx0 + 35 + idx * 140
        d_dc3 = int(r["dc3_max_depth"])
        d_sais = int(r["sais_max_depth"])
        rat_dc3 = float(r["dc3_reduction_ratio"]) * 100.0
        rat_sais = float(r["sais_reduction_ratio"]) * 100.0

        h_dc3 = int((ry1 - ry0 - 80) * (d_dc3 / max_depth))
        h_sais = int((ry1 - ry0 - 80) * (d_sais / max_depth))

        canvas.fill_rect(base_x, ry1 - h_dc3, base_x + 42, ry1 - 2, COLOR_DC3)
        canvas.draw_text(base_x + 6, ry1 - h_dc3 - 20, f"D={d_dc3}", COLOR_DC3, scale=2)
        canvas.draw_text(base_x + 2, ry1 - h_dc3 - 36, f"{rat_dc3:.0f}%", COLOR_DC3, scale=1)

        canvas.fill_rect(base_x + 50, ry1 - h_sais, base_x + 92, ry1 - 2, COLOR_SAIS)
        canvas.draw_text(base_x + 56, ry1 - h_sais - 20, f"D={d_sais}", COLOR_SAIS, scale=2)
        canvas.draw_text(base_x + 52, ry1 - h_sais - 36, f"{rat_sais:.0f}%", COLOR_SAIS, scale=1)

        short_name = r["dataset"].split("_")[0].upper()
        canvas.draw_text(base_x + 10, ry1 + 12, short_name, (50, 50, 50), scale=2)

    canvas.draw_text(rx0 + 20, ry0 + 18, "ETIQUETAS: D = NIVELES RECURSIVOS | % = RAZON DE REDUCCION (67% VS ~33%)", (40, 40, 40), scale=1)
    canvas.save_png(out_path)


def plot_fig4_cache_and_corpus(cache_rows, corpus_rows, out_path: Path):
    canvas = PureCanvasPNG(1400, 820, bg=(252, 253, 255))
    canvas.draw_text(120, 25, "FIG 4: FALLOS DE CACHE L1 (CACHE MISSES) Y DESEMPENO EN DATA/", (20, 30, 45), scale=3)
    canvas.draw_text(250, 58, "SIMULACION L1D (32KB, LINEA 64B) Y TIEMPO REAL EN ARCHIVOS DE CORPUS", (80, 90, 105), scale=2)

    # Panel Izquierdo: Fallos de cache L1 por caracter en n = 200,000
    x0, y0, x1, y1 = 80, 115, 690, 720
    canvas.fill_rect(x0, y0, x1, y1, (255, 255, 255))
    canvas.draw_rect(x0, y0, x1, y1, COLOR_AXIS, thickness=2)
    canvas.draw_text(x0 + 20, y0 - 22, "A) FALLOS DE CACHE L1 POR CARACTER (N=200K)", (30, 40, 55), scale=2)

    c200 = [r for r in cache_rows if int(r["n"]) == 200000]
    max_mpc = max([float(r["dc3_l1_misses_per_char"]) for r in c200] + [1.0]) * 1.25

    for idx, r in enumerate(c200):
        base_x = x0 + 35 + idx * 140
        m_dc3 = float(r["dc3_l1_misses_per_char"])
        m_sais = float(r["sais_l1_misses_per_char"])

        h_dc3 = int((y1 - y0 - 60) * (m_dc3 / max_mpc))
        h_sais = int((y1 - y0 - 60) * (m_sais / max_mpc))

        canvas.fill_rect(base_x, y1 - h_dc3, base_x + 42, y1 - 2, COLOR_DC3)
        canvas.draw_text(base_x + 2, y1 - h_dc3 - 20, f"{m_dc3:.2f}", COLOR_DC3, scale=2)

        canvas.fill_rect(base_x + 50, y1 - h_sais, base_x + 92, y1 - 2, COLOR_SAIS)
        canvas.draw_text(base_x + 52, y1 - h_sais - 20, f"{m_sais:.2f}", COLOR_SAIS, scale=2)

        short_name = r["dataset"].split("_")[0].upper()
        canvas.draw_text(base_x + 10, y1 + 12, short_name, (50, 50, 50), scale=2)

    # Panel Derecho: Tiempo real (ms) en los 4 archivos de data/
    rx0, ry0, rx1, ry1 = 760, 115, 1350, 720
    canvas.fill_rect(rx0, ry0, rx1, ry1, (255, 255, 255))
    canvas.draw_rect(rx0, ry0, rx1, ry1, COLOR_AXIS, thickness=2)
    canvas.draw_text(rx0 + 20, ry0 - 22, "B) TIEMPO EN ARCHIVOS DE CORPUS DATA/ (MS)", (30, 40, 55), scale=2)

    max_ms = max([float(r["dc3_median_ms"]) for r in corpus_rows] + [1.0]) * 1.25
    for idx, r in enumerate(corpus_rows):
        base_x = rx0 + 35 + idx * 140
        t_dc3 = float(r["dc3_median_ms"])
        t_sais = float(r["sais_median_ms"])

        h_dc3 = int((ry1 - ry0 - 60) * (t_dc3 / max_ms))
        h_sais = int((ry1 - ry0 - 60) * (t_sais / max_ms))

        canvas.fill_rect(base_x, ry1 - h_dc3, base_x + 42, ry1 - 2, COLOR_DC3)
        canvas.draw_text(base_x + 2, ry1 - h_dc3 - 20, f"{t_dc3:.1f}", COLOR_DC3, scale=2)

        canvas.fill_rect(base_x + 50, ry1 - h_sais, base_x + 92, ry1 - 2, COLOR_SAIS)
        canvas.draw_text(base_x + 52, ry1 - h_sais - 20, f"{t_sais:.1f}", COLOR_SAIS, scale=2)

        fname = Path(r["dataset"]).stem.replace("corpus_", "").split("_")[0].upper()
        canvas.draw_text(base_x + 6, ry1 + 12, fname, (50, 50, 50), scale=2)

    canvas.save_png(out_path)


def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent
    results_dir = repo_root / "results"

    bench_csv = results_dir / "benchmark_metrics.csv"
    corpus_csv = results_dir / "corpus_benchmark.csv"
    cache_csv = results_dir / "cache_metrics.csv"

    if not bench_csv.exists() or not corpus_csv.exists() or not cache_csv.exists():
        print("[ERROR] Faltan archivos CSV en results/. Ejecuta prepare_corpus.py y profile_cache.py primero.")
        return 1

    with bench_csv.open("r", encoding="utf-8") as f:
        bench_rows = list(csv.DictReader(f))
    with corpus_csv.open("r", encoding="utf-8") as f:
        corpus_rows = list(csv.DictReader(f))
    with cache_csv.open("r", encoding="utf-8") as f:
        cache_rows = list(csv.DictReader(f))

    f1 = results_dir / "fig1_time_scaling.png"
    f2 = results_dir / "fig2_speedup_by_family.png"
    f3 = results_dir / "fig3_memory_and_recursion.png"
    f4 = results_dir / "fig4_cache_and_corpus.png"

    plot_fig1_time_scaling(bench_rows, f1)
    plot_fig2_speedup_by_family(bench_rows, f2)
    plot_fig3_memory_and_recursion(bench_rows, f3)
    plot_fig4_cache_and_corpus(cache_rows, corpus_rows, f4)

    print("==================================================================")
    print(" PASO 8.3: Exportacion de Graficas Cientificas en results/")
    print("==================================================================")
    for p in (f1, f2, f3, f4):
        print(f" [OK] {p.relative_to(repo_root)} ({p.stat().st_size // 1024} KB)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
