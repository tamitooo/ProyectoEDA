#!/usr/bin/env python3
"""
scripts/prepare_corpus.py
Genera los 4 archivos de corpus experimentales en data/ (texto aleatorio, genoma ADN,
codigo fuente C++ real del proyecto y cadenas altamente periodicas) y ejecuta el
binario compilado build/benchmark.exe para exportar results/corpus_benchmark.csv.
"""

from pathlib import Path
import random
import subprocess
import sys


def build_random_corpus(target_len: int, seed: int = 42) -> str:
    rng = random.Random(seed)
    alphabet = [chr(33 + i) for i in range(64)]
    return "".join(rng.choice(alphabet) for _ in range(target_len))


def build_dna_corpus(target_len: int, seed: int = 1337) -> str:
    rng = random.Random(seed)
    bases = ["A", "C", "G", "T"]
    motifs = [
        "GATTACAGATTACATGCA",
        "TATATATATATATATATA",
        "CGCGCGCGCGCGCGCGCG",
        "ATGAAAGCCCTTTGGGAAATTTCCC",
    ]
    chunks = []
    current_len = 0
    while current_len < target_len:
        if rng.random() < 0.18:
            m = rng.choice(motifs)
            chunks.append(m)
            current_len += len(m)
        else:
            b = rng.choice(bases)
            chunks.append(b)
            current_len += 1
    return "".join(chunks)[:target_len]


def build_source_code_corpus(repo_root: Path, target_len: int, seed: int = 2026) -> str:
    rng = random.Random(seed)
    source_files = [
        repo_root / "include" / "naive_sa.hpp",
        repo_root / "include" / "dc3.hpp",
        repo_root / "src" / "dc3.cpp",
        repo_root / "include" / "sais.hpp",
        repo_root / "src" / "sais.cpp",
        repo_root / "tests" / "test_runner.cpp",
        repo_root / "bench" / "benchmark.cpp",
        repo_root / "scripts" / "prepare_corpus.py",
        repo_root / "scripts" / "profile_cache.py",
        repo_root / "scripts" / "plot_results.py",
    ]
    chunks = []
    current_len = 0
    for p in source_files:
        if p.exists():
            text = p.read_text(encoding="utf-8", errors="ignore")
            chunks.append(text)
            current_len += len(text)

    # Generar modulos C++ variados (estados de parser LR(1), AST y rutinas de tabla de simbolos)
    # sin repetir de forma identica el mismo bloque completo.
    types = ["int", "std::size_t", "uint32_t", "int64_t", "double", "bool"]
    ops = ["+", "-", "^", "&", "|", "*"]
    tokens = ["TOK_IDENT", "TOK_NUM", "TOK_LPAREN", "TOK_RPAREN", "TOK_ASSIGN", "TOK_SEMI", "TOK_EOF"]
    mod_id = 0
    while current_len < target_len:
        t_ret = rng.choice(types)
        t_arg = rng.choice(types)
        op1, op2 = rng.choice(ops), rng.choice(ops)
        tok_a, tok_b = rng.choice(tokens), rng.choice(tokens)
        k1 = rng.randint(3, 251)
        k2 = rng.randint(100, 9999)
        snippet = (
            f"\n// Modulo Sintetico de Analisis Lexico/Sintactico #{mod_id}\n"
            f"namespace lexer_unit_{mod_id} {{\n"
            f"static constexpr int STATE_BASE_{mod_id} = {k2};\n"
            f"inline {t_ret} transition_step_{mod_id}({t_arg} sym, int state, const int* bkt) {{\n"
            f"    int next_s = (state {op1} {k1}) {op2} static_cast<int>(sym);\n"
            f"    if (sym == {mod_id % 127} && next_s > 0) {{\n"
            f"        return static_cast<{t_ret}>(bkt[next_s & 0xFF] + STATE_BASE_{mod_id});\n"
            f"    }} else if (sym == {(mod_id * 7) % 127}) {{\n"
            f"        for (int i = 0; i < {(mod_id % 16) + 4}; ++i) {{\n"
            f"            next_s = (next_s * 31 + i) ^ {k1};\n"
            f"        }}\n"
            f"        return static_cast<{t_ret}>(next_s); // {tok_a} -> {tok_b}\n"
            f"    }}\n"
            f"    return static_cast<{t_ret}>(0);\n"
            f"}}\n"
            f"}} // namespace lexer_unit_{mod_id}\n"
        )
        chunks.append(snippet)
        current_len += len(snippet)
        mod_id += 1

    return "".join(chunks)[:target_len]


def build_periodic_corpus(target_len: int) -> str:
    # Palabra de Fibonacci combinada con patrones clasicos de los papers
    f0, f1 = "b", "a"
    while len(f1) < 4096:
        f0, f1 = f1, f1 + f0
    pattern = f1 + "_mississippi_mmiissiissiippii_abracadabra_"
    repeats = (target_len // len(pattern)) + 2
    return (pattern * repeats)[:target_len]


def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent
    data_dir = repo_root / "data"
    results_dir = repo_root / "results"
    data_dir.mkdir(parents=True, exist_ok=True)
    results_dir.mkdir(parents=True, exist_ok=True)

    target_size = 1_000_000
    corpus_specs = [
        ("corpus_random.txt", build_random_corpus(target_size)),
        ("corpus_dna.fasta", build_dna_corpus(target_size)),
        ("corpus_source_code.txt", build_source_code_corpus(repo_root, target_size)),
        ("corpus_periodic.txt", build_periodic_corpus(target_size)),
    ]

    saved_paths = []
    print("==================================================================")
    print(" PASO 8.1: Generacion de Corpus Experimentales en data/")
    print("==================================================================")
    for filename, content in corpus_specs:
        out_path = data_dir / filename
        out_path.write_text(content, encoding="utf-8", newline="\n")
        unique_chars = len(set(content))
        print(f" [OK] {out_path.relative_to(repo_root)} -> {len(content)} bytes (|Sigma| = {unique_chars})")
        saved_paths.append(str(out_path.relative_to(repo_root)).replace("\\", "/"))

    bench_exe = repo_root / "build" / ("benchmark.exe" if sys.platform.startswith("win") else "benchmark")
    if not bench_exe.exists():
        print(f"[ERROR] No se encontro el ejecutable {bench_exe}. Compilalo primero.")
        return 1

    out_csv = "results/corpus_benchmark.csv"
    cmd = [str(bench_exe), "--only-files", out_csv] + saved_paths
    print("\nEjecutando benchmark sobre los archivos de data/...")
    proc = subprocess.run(cmd, cwd=str(repo_root), check=False)
    return proc.returncode


if __name__ == "__main__":
    raise SystemExit(main())
