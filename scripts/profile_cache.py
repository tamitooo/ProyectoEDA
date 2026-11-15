#!/usr/bin/env python3
"""
scripts/profile_cache.py
Ejecuta el simulador de jerarquia de cache L1D (32 KB, 8-way, lineas de 64B) y
L2 (256 KB, 8-way, lineas de 64B) integrado en build/benchmark.exe y analiza
la localidad espacial/temporal de DC3 frente a SA-IS.
"""

import csv
from pathlib import Path
import subprocess
import sys


def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent
    results_dir = repo_root / "results"
    results_dir.mkdir(parents=True, exist_ok=True)

    bench_exe = repo_root / "build" / ("benchmark.exe" if sys.platform.startswith("win") else "benchmark")
    if not bench_exe.exists():
        print(f"[ERROR] No se encontro el ejecutable {bench_exe}. Compilalo primero.")
        return 1

    out_csv = "results/cache_metrics.csv"
    cmd = [str(bench_exe), "--profile-cache", out_csv]
    proc = subprocess.run(cmd, cwd=str(repo_root), check=False)
    if proc.returncode != 0:
        return proc.returncode

    csv_file = repo_root / out_csv
    if not csv_file.exists():
        return 1

    with csv_file.open("r", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    print("\nResumen de Localidad Espacial (n = 200,000):")
    print("----------------------------------------------------------------------------------------------------")
    print("Distribucion  | Accesos Indirectos (DC3 vs SAIS) | Fallos L1 / char (DC3 vs SAIS) | Reduccion Fallos")
    print("----------------------------------------------------------------------------------------------------")
    for r in rows:
        if int(r["n"]) == 200000:
            ds = r["dataset"]
            dc3_ind = float(r["dc3_indirect_pct"])
            sais_ind = float(r["sais_indirect_pct"])
            dc3_mpc = float(r["dc3_l1_misses_per_char"])
            sais_mpc = float(r["sais_l1_misses_per_char"])
            ratio = float(r["l1_miss_ratio_dc3_over_sais"])
            print(
                f"{ds:<13} | {dc3_ind:>12.2f}% vs {sais_ind:>11.2f}%   | "
                f"{dc3_mpc:>11.2f} vs {sais_mpc:>11.2f}    | {ratio:>12.2f}x menos"
            )
    print("----------------------------------------------------------------------------------------------------")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
