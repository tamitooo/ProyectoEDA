# Construcción de Suffix Arrays en Tiempo Lineal $\mathcal{O}(n)$: DC3 (Skew) vs. SA-IS

**Curso:** CS3014 — Estructuras de Datos Avanzadas (UTEC, 2026-2)  
**Estado del Proyecto:** ✅ Completado y Validado al 100% (Etapas 1 a 5)  
**Estándar:** C++17 (`-O3 -Wall -Wextra -Wpedantic`) + Pipeline Experimental en Python 3  

---

## 1. Descripción General

Este repositorio contiene una implementación desde cero en **C++17** y una evaluación experimental exhaustiva de los dos algoritmos más influyentes en la historia de la construcción de **Arreglos de Sufijos (*Suffix Arrays*, SA)** en tiempo lineal $\mathcal{O}(n)$:

1. **DC3 / Algoritmo Skew** — *Kärkkäinen & Sanders (ICALP 2003)*: Basado en división algebraica regular de las posiciones por residuo módulo 3 ($i \bmod 3 \neq 0$ vs. $i \bmod 3 = 0$), ordenamiento por tripletas con Radix Sort y fusión en $\mathcal{O}(1)$ por comparación mediante el arreglo inverso $\overline{SA}^{12}$.
2. **SA-IS (*Suffix Array Induced Sorting*)** — *Nong, Zhang & Chan (IEEE DCC 2009)*: Basado en la estructura léxica intrínseca de la cadena mediante la clasificación lineal de sufijos en tipos **L (*Larger*)** y **S (*Smaller*)**, el muestreo disperso de posiciones **LMS (*Leftmost S-type*)** ($\alpha \le 0.5$) y la propagación secuencial por **ordenación inducida** (`induceSAl` e `induceSAs`).
3. **Baseline Ingenuo (`Naive SA`)**: Construcción por ordenamiento directo de sufijos con `std::sort` y comparaciones `std::string_view` en $\mathcal{O}(n^2 \log n)$, utilizado como oráculo de corrección y punto de referencia asintótico.

---

## 2. Estructura del Repositorio

```text
ProyectoEDA/
├── CMakeLists.txt                     # Configuración de compilación CMake (C++17)
├── README.md                          # Documentación principal del repositorio
├── include/
│   ├── naive_sa.hpp                   # Baseline O(n^2 log n), conversión a enteros + centinela y verificador SA
│   ├── dc3.hpp                        # Interfaz pública, subrutinas internas e instrumentación de DC3 (Skew)
│   └── sais.hpp                       # Interfaz pública, subrutinas L/S/LMS e instrumentación de SA-IS
├── src/
│   ├── dc3.cpp                        # Implementación completa de DC3 (leq, radixPass, dc3Phase1, suffixArray)
│   └── sais.cpp                       # Implementación completa de SA-IS (classifyLS, getBuckets, induceSAl/s, sais_core)
├── tests/
│   └── test_runner.cpp                # Suite de 6 pruebas unitarias y de validación cruzada (Naive == DC3 == SA-IS)
├── bench/
│   └── benchmark.cpp                  # Motor de benchmarks (tiempo, memoria RAM, recursión, corpus y simulador L1/L2)
├── scripts/
│   ├── prepare_corpus.py              # Generador reproducible de los 4 corpus reales en data/ y corrida de benchmark
│   ├── profile_cache.py               # Orquestador del simulador de jerarquía de caché L1D (32KB) y L2 (256KB)
│   └── plot_results.py                # Generador autónomo de las 4 figuras científicas en alta resolución (PNG)
├── data/
│   ├── corpus_random.txt              # Corpus ASCII aleatorio uniforme (250,000 bytes, |Sigma| = 64)
│   ├── corpus_dna.fasta               # Corpus genómico FASTA con repeticiones en tándem (250,000 bp, |Sigma| = 4)
│   ├── corpus_source_code.txt         # Corpus de código fuente C/C++ real concatenado (257,069 bytes)
│   └── corpus_periodic.txt            # Corpus altamente periódico / Fibonacci / repeticiones largas (250,000 bytes)
├── results/
│   ├── benchmark_metrics.csv          # Métricas de escalabilidad sintética (n = 2,000 a n = 500,000)
│   ├── corpus_benchmark.csv           # Métricas sobre los 4 archivos de corpus en data/
│   ├── cache_metrics.csv              # Métricas de simulación de caché L1D/L2 y accesos indirectos (n = 200,000)
│   ├── fig1_time_scaling.png          # Figura 1: Escalabilidad de tiempo de ejecución (4 paneles)
│   ├── fig2_speedup_by_family.png     # Figura 2: Speedup (T_DC3 / T_SAIS) en función de n
│   ├── fig3_memory_and_recursion.png  # Figura 3: Memoria pico (B/char), razón de reducción y profundidad recursiva
│   └── fig4_cache_and_corpus.png      # Figura 4: Fallos de caché L1/L2 y tiempos en corpus reales
└── docs/
    ├── PROPUESTA_Y_RUTA_APRENDIZAJE.md # Plan maestro del proyecto dividido en 5 etapas pedagógicas
    ├── FUNDAMENTOS_PAPERS_BASE.md     # Análisis matemático detallado de Kärkkäinen & Sanders (2003) y Nong et al. (2009)
    ├── INFORME_TECNICO_FINAL.md       # Artículo académico final con tablas experimentales y figuras integradas
    └── GUIA_EXPOSICION_Y_DEFENSA.md   # Guion minuto a minuto para la defensa oral y banco de 8 preguntas de jurado
```

---

## 3. Instrucciones de Compilación y Ejecución (Windows PowerShell)

### 3.1. Compilación de Binarios C++17 (`test_runner.exe` y `benchmark.exe`)

Puedes compilar directamente con `g++` (MinGW-w64 / MSYS2) en modo optimizado `-O3`:

```powershell
# Crear directorio de compilación si no existe
if (!(Test-Path build)) { New-Item -ItemType Directory -Path build | Out-Null }

# 1. Compilar el ejecutable de pruebas unitarias (test_runner.exe)
g++ -std=c++17 -O3 -Wall -Wextra -Wpedantic -Iinclude tests/test_runner.cpp src/dc3.cpp src/sais.cpp -o build/test_runner.exe

# 2. Compilar el motor de benchmarks y perfilado de caché (benchmark.exe)
g++ -std=c++17 -O3 -Wall -Wextra -Wpedantic -Iinclude bench/benchmark.cpp src/dc3.cpp src/sais.cpp -o build/benchmark.exe -lpsapi
```

*(Alternativamente, si utilizas CMake: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`).*

---

### 3.2. Ejecución de las Pruebas Unitarias y de Corrección

```powershell
.\build\test_runner.exe
```

Ejecuta las **6 suites de validación** correspondientes a las Etapas 1, 2 y 3:
1. `[TEST Etapa 1]` Validación de `build_naive_sa` sobre `"mississippi"` y `"mmiissiissiippii"`.
2. `[TEST Paso 2]` Validación de comparadores lexicográficos `leq` y estabilidad de `radixPass`.
3. `[TEST Paso 3]` Validación de `dc3Phase1` (muestreo $\bmod 3 \neq 0$, nombramiento en `s12` y `SA12`).
4. `[TEST Paso 4]` Validación exhaustiva de `build_dc3_sa` en casos borde, cadenas periódicas y pruebas aleatorias contra `Naive`.
5. `[TEST Paso 5]` Validación de `classifyLS`, `isLMS`, `getBuckets` e inducción parcial de subcadenas LMS.
6. `[TEST Paso 6]` Validación exhaustiva de `build_sais_sa` verificando identidad exacta `SA_Naive == SA_DC3 == SA_SAIS`.

---

### 3.3. Ejecución del Pipeline Experimental Completo (Etapa 4)

```powershell
# 1. Ejecutar el benchmark de escalabilidad sintética (n = 2,000 a 500,000) -> results/benchmark_metrics.csv
.\build\benchmark.exe

# 2. Generar los 4 corpus en data/ y medir rendimiento sobre archivos reales -> results/corpus_benchmark.csv
python scripts/prepare_corpus.py

# 3. Ejecutar la simulación de caché L1D (32 KB) y L2 (256 KB) en n = 200,000 -> results/cache_metrics.csv
python scripts/profile_cache.py

# 4. Generar las 4 figuras científicas PNG en alta resolución dentro de results/
python scripts/plot_results.py
```

---

## 4. Resumen de Resultados Experimentales

### 4.1. Escalabilidad en el Tamaño Máximo ($n = 500{,}000$)

Datos extraídos de [`results/benchmark_metrics.csv`](results/benchmark_metrics.csv):

| Familia de Cadena ($n = 500{,}000$) | Tiempo DC3 (ms) | Tiempo SA-IS (ms) | Speedup (`DC3 / SA-IS`) | Razón Reducción ($\alpha_{\text{DC3}}$ / $\alpha_{\text{SA-IS}}$) | Profundidad Recursión (DC3 / SA-IS) | Memoria Pico DC3 (B/char) | Memoria Pico SA-IS 32b / Compact (B/char) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **`random_ascii`** ($|\Sigma|=64$) | `25.59` | **`21.98`** | **`1.16x`** | `0.667` / `0.331` | `1` / `1` | `20.18` | `14.46` / **`6.30`** |
| **`dna_genome`** ($|\Sigma|=4$) | `34.43` | **`17.01`** | **`2.02x`** | `0.667` / `0.355` | `4` / `4` | `23.21` | `14.48` / **`5.30`** |
| **`source_code`** (C/C++) | `36.02` | **`16.46`** | **`2.19x`** | `0.667` / `0.337` | `4` / `4` | `23.21` | `14.15` / **`5.27`** |
| **`periodic_str`** (Periódica) | `19.84` | **`11.52`** | **`1.72x`** | `0.667` / `0.308` | `11` / `3` | `23.92` | `13.64` / **`5.18`** |

> **Nota sobre el Baseline Ingenuo (`Naive SA`):** En `periodic_str` con apenas $n = 10{,}000$, `Naive SA` tarda **`153.91 ms`**, mientras que DC3 toma `0.41 ms` ($377\times$ más rápido) y SA-IS toma **`0.22 ms`** ($700\times$ más rápido).

---

### 4.2. Rendimiento sobre Archivos de Corpus en `data/`

Datos extraídos de [`results/corpus_benchmark.csv`](results/corpus_benchmark.csv):

| Archivo de Corpus | Tamaño ($n$) | Tiempo DC3 (ms) | Tiempo SA-IS (ms) | Speedup (`DC3 / SA-IS`) | Razón Reducción ($\alpha_{\text{SA-IS}}$) | Profundidad Recursión (DC3 / SA-IS) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| [`data/corpus_random.txt`](data/corpus_random.txt) | `250,000` | `11.67` | **`10.37`** | **`1.13x`** | `0.331` | `1` / `1` |
| [`data/corpus_dna.fasta`](data/corpus_dna.fasta) | `250,000` | `16.06` | **`8.17`** | **`1.97x`** | `0.348` | `4` / `3` |
| [`data/corpus_source_code.txt`](data/corpus_source_code.txt) | `257,069` | `18.88` | **`7.54`** | **`2.51x`** | **`0.265`** | `10` / `9` |
| [`data/corpus_periodic.txt`](data/corpus_periodic.txt) | `250,000` | `12.42` | **`7.48`** | **`1.66x`** | `0.381` | `11` / `8` |

---

### 4.3. Perfilado de Localidad de Caché L1D (32 KB) y L2 (256 KB) ($n = 200{,}000$)

Datos extraídos de [`results/cache_metrics.csv`](results/cache_metrics.csv):

| Familia ($n = 200{,}000$) | % Accesos Indirectos (DC3 vs. SA-IS) | Tasa de Fallos L1 (DC3 vs. SA-IS) | Fallos L1 por Carácter (DC3 vs. SA-IS) | Reducción de Fallos L1 en SA-IS |
| :--- | :---: | :---: | :---: | :---: |
| **`random_ascii`** | `53.24%` vs. **`35.72%`** | `33.28%` vs. **`24.85%`** | `18.40` vs. **`14.86`** | **`1.24x` menos fallos** |
| **`dna_genome`** | `54.92%` vs. **`36.54%`** | `27.76%` vs. **`17.31%`** | `23.26` vs. **`11.85`** | **`1.96x` menos fallos** |
| **`source_code`** | `55.47%` vs. **`36.22%`** | `29.21%` vs. **`19.05%`** | `24.23` vs. **`12.57`** | **`1.93x` menos fallos** |
| **`periodic_str`** | `57.18%` vs. **`36.31%`** | `21.67%` vs. **`16.70%`** | `19.74` vs. **`10.24`** | **`1.93x` menos fallos** |

---

## 5. Visualizaciones Generadas (`results/`)

1. **[`results/fig1_time_scaling.png`](results/fig1_time_scaling.png)**: Curvas de escalabilidad temporal $T(n)$ en las 4 familias de cadenas ($n = 2{,}000$ a $500{,}000$).
2. **[`results/fig2_speedup_by_family.png`](results/fig2_speedup_by_family.png)**: Evolución del factor de aceleración $\text{Speedup} = T_{\text{DC3}} / T_{\text{SA-IS}}$ por familia.
3. **[`results/fig3_memory_and_recursion.png`](results/fig3_memory_and_recursion.png)**: Comparativa de memoria RAM pico (`B/char`), razón de reducción $\alpha$ y profundidad máxima de recursión.
4. **[`results/fig4_cache_and_corpus.png`](results/fig4_cache_and_corpus.png)**: Fallos de caché L1/L2 por carácter y tiempos de ejecución sobre los 4 archivos de corpus reales en `data/`.

---

## 6. Documentación Académica y Defensa del Proyecto (`docs/`)

- 📄 **[Informe Técnico Final (`docs/INFORME_TECNICO_FINAL.md`)](docs/INFORME_TECNICO_FINAL.md)**: Artículo académico completo (estilo IEEE/ACM) con demostraciones teóricas, detalles de implementación in-place, tablas experimentales, análisis de caché y figuras integradas.
- 🎤 **[Guía de Exposición y Defensa Oral (`docs/GUIA_EXPOSICION_Y_DEFENSA.md`)](docs/GUIA_EXPOSICION_Y_DEFENSA.md)**: Estructura sugerida de diapositivas minuto a minuto, los 5 puntos técnicos clave del código C++17 y un banco de 8 preguntas difíciles de jurado con sus respuestas exactas.
- 📚 **[Fundamentos de los Papers Base (`docs/FUNDAMENTOS_PAPERS_BASE.md`)](docs/FUNDAMENTOS_PAPERS_BASE.md)**: Desglose teórico paso a paso de Kärkkäinen & Sanders (2003) y Nong, Zhang & Chan (2009).
- 🗺️ **[Propuesta y Ruta de Aprendizaje (`docs/PROPUESTA_Y_RUTA_APRENDIZAJE.md`)](docs/PROPUESTA_Y_RUTA_APRENDIZAJE.md)**: Planificación pedagógica original en 5 etapas.
