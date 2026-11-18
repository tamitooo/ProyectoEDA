# Construcción de Suffix Arrays en Tiempo Lineal $\mathcal{O}(n)$: DC3 (Skew) vs. SA-IS

**Curso:** CS3014 — Estructuras de Datos Avanzadas (UTEC, 2026-2)  
**Estado del Proyecto:** Completado y Verificado (Etapas 1 a 5)  
**Estándar y Entorno:** C++17 (`g++ -std=c++17 -O3 -Wall -Wextra -Wpedantic`) · Windows 11 x86_64 · AMD Ryzen 7 7435HS (16 MB L3) · 24 GB RAM  

---

## 1. Descripción General

Este repositorio presenta la adaptación en **C++17** y la evaluación experimental comparativa de dos algoritmos fundamentales para la construcción de **Arreglos de Sufijos (*Suffix Arrays*, SA)** en tiempo lineal $\mathcal{O}(n)$:

1. **DC3 / Algoritmo Skew** — *Kärkkäinen & Sanders (ICALP 2003)*: Basado en división algebraica regular de las posiciones por residuo módulo 3 ($i \bmod 3 \neq 0$ vs. $i \bmod 3 = 0$), ordenamiento por tripletas con Radix Sort y fusión en $\mathcal{O}(1)$ por comparación mediante el arreglo inverso $\overline{SA}^{12}$.
2. **SA-IS (*Suffix Array Induced Sorting*)** — *Nong, Zhang & Chan (IEEE DCC 2009)*: Basado en la estructura léxica de la cadena mediante la clasificación lineal de sufijos en tipos **L (*Larger*)** y **S (*Smaller*)**, el muestreo de posiciones **LMS (*Leftmost S-type*)** ($\alpha \le 0.5$) y la propagación secuencial por **ordenación inducida** (`induceSAl` e `induceSAs`).
3. **Baseline Ingenuo (`Naive SA`)**: Construcción por ordenamiento directo de sufijos con `std::sort` en $\mathcal{O}(n^2 \log n)$, utilizado como oráculo de corrección y punto de referencia asintótico.

> **Nota sobre autoría de referencia y adaptaciones:** Las implementaciones de DC3 y SA-IS están **basadas en las implementaciones de referencia del Apéndice A de Kärkkäinen & Sanders (2003) y de Nong, Zhang & Chan (2009)**, adaptadas a C++17 utilizando contenedores RAII (`std::vector<int>`), soporte completo para el alfabeto binario `[0..255]` (desplazando cada byte `+1` al rango `[1..256]` con `K = 256` y reservando `0` exclusivamente para el centinela) e instrumentación analítica de recursión, memoria y caché.

---

## 2. Estructura del Repositorio

```text
ProyectoEDA/
├── CMakeLists.txt                     # Configuración de compilación CMake (C++17)
├── README.md                          # Documentación principal del repositorio
├── include/
│   ├── naive_sa.hpp                   # Baseline O(n^2 log n), mapeo [1..256] + centinela 0 y verificador SA
│   ├── dc3.hpp                        # Interfaz pública, subrutinas internas e instrumentación de DC3 (Skew)
│   └── sais.hpp                       # Interfaz pública, subrutinas L/S/LMS e instrumentación de SA-IS
├── src/
│   ├── dc3.cpp                        # Adaptación C++17 de DC3 (leq, radixPass, dc3Phase1, suffixArray)
│   └── sais.cpp                       # Adaptación C++17 de SA-IS (classifyLS, getBuckets, induceSAl/s, sais_core)
├── tests/
│   └── test_runner.cpp                # Suite de 6 pruebas unitarias (#undef NDEBUG), casos con '\0' y 1,000 binarias
├── bench/
│   └── benchmark.cpp                  # Motor de benchmarks (n hasta 2,000,000, corpus de 1 MB y simulador L1/L2)
├── scripts/
│   ├── prepare_corpus.py              # Generador reproducible de los 4 corpus de 1 MB en data/ y corrida de benchmark
│   ├── profile_cache.py               # Orquestador del simulador de jerarquía de caché L1D (32KB) y L2 (256KB)
│   └── plot_results.py                # Generador autónomo de las 4 figuras científicas en alta resolución (PNG)
├── data/
│   ├── corpus_random.txt              # Corpus ASCII aleatorio uniforme (1,000,000 bytes, |Sigma| = 64)
│   ├── corpus_dna.fasta               # Corpus genómico FASTA con repeticiones en tándem (1,000,000 bp, |Sigma| = 4)
│   ├── corpus_source_code.txt         # Corpus C/C++ para preprocesamiento léxico y parsers LR(1) (1,000,000 B, |Sigma| = 95)
│   └── corpus_periodic.txt            # Corpus altamente periódico / Fibonacci / repeticiones largas (1,000,000 bytes, |Sigma| = 10)
├── results/
│   ├── benchmark_metrics.csv          # Métricas de escalabilidad sintética (n = 2,000 a n = 2,000,000)
│   ├── corpus_benchmark.csv           # Métricas sobre los 4 archivos de corpus de 1 MB en data/
│   ├── cache_metrics.csv              # Métricas de simulación de caché L1D/L2 y accesos indirectos (n = 200,000)
│   ├── fig1_time_scaling.png          # Figura 1: Escalabilidad de tiempo de ejecución (4 paneles, hasta n = 2M)
│   ├── fig2_speedup_by_family.png     # Figura 2: Speedup (T_DC3 / T_SAIS) en función de n
│   ├── fig3_memory_and_recursion.png  # Figura 3: Memoria analítica (B/char), razón de reducción y profundidad recursiva
│   └── fig4_cache_and_corpus.png      # Figura 4: Fallos de caché L1/L2 y tiempos en corpus de 1 MB
└── docs/
    ├── PROPUESTA_Y_RUTA_APRENDIZAJE.md # Plan maestro del proyecto dividido en 5 etapas pedagógicas
    ├── FUNDAMENTOS_PAPERS_BASE.md     # Análisis matemático detallado de Kärkkäinen & Sanders (2003) y Nong et al. (2009)
    ├── INFORME_TECNICO_FINAL.md       # Artículo académico final con tablas experimentales y figuras integradas
    └── GUIA_EXPOSICION_Y_DEFENSA.md   # Guion minuto a minuto para la defensa oral y banco de 8 preguntas de jurado
```

---

## 3. Instrucciones de Compilación y Ejecución (Windows PowerShell)

### 3.1. Compilación de Binarios C++17 (`test_runner.exe` y `benchmark.exe`)

Puedes compilar directamente con `g++` (MinGW-w64 / MSYS2) en modo optimizado `-O3` (tanto `test_runner.cpp` como `benchmark.cpp` incluyen `#undef NDEBUG` antes de `<cassert>` para que las verificaciones `assert(...)` nunca se desactiven, incluso si se compila con CMake en modo `Release`):

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
4. `[TEST Paso 4]` Validación de `build_dc3_sa` en trazas del paper y casos borde contra `Naive`.
5. `[TEST Paso 5]` Validación de `classifyLS`, `isLMS`, `getBuckets` e inducción parcial de subcadenas LMS.
6. `[TEST Paso 6]` Validación cruzada (`SA_Naive == SA_DC3 == SA_SAIS`) en **35 escenarios deterministas** (incluyendo cadenas con bytes nulos `\0` y los 256 valores `[0..255]`) más **1,000 cadenas binarias aleatorias con `\0`**.

---

### 3.3. Ejecución del Pipeline Experimental Completo (Etapa 4)

```powershell
# 1. Ejecutar el benchmark de escalabilidad sintética (n = 2,000 a 2,000,000) -> results/benchmark_metrics.csv
.\build\benchmark.exe

# 2. Generar los 4 corpus de 1 MB en data/ y medir rendimiento -> results/corpus_benchmark.csv
python scripts/prepare_corpus.py

# 3. Ejecutar la simulación de caché L1D (32 KB) y L2 (256 KB) en n = 200,000 -> results/cache_metrics.csv
python scripts/profile_cache.py

# 4. Generar las 4 figuras científicas PNG en alta resolución dentro de results/
python scripts/plot_results.py
```

---

## 4. Resumen de Resultados Experimentales

> **Entorno y Metodología de Medición:**  
> - **Hardware:** Windows 11 x86_64, CPU **AMD Ryzen 7 7435HS (16 MB de caché L3)** y **24 GB de RAM**.  
> - **Memoria Analítica Total (`B/char` en 32b)** (`dc3_bytes_per_char` y `sais_bytes_per_char`): Mide de forma **analítica exacta** el pico de memoria reservado en el heap por los vectores `std::vector<int>` del algoritmo (`size() * sizeof(int)`) a través de los niveles recursivos. SA-IS ahorra entre un **`26% y 43%` global** (`38%–43%` en textos estructurados para $n = 2{,}000{,}000$, y `33%–43%` en todo el rango de textos estructurados). En **memoria auxiliar (`peak_aux_kb`)** en $n = 2{,}000{,}000$, el ahorro de SA-IS es de **entre `44%` (`random_ascii`) y `65%` (`periodic_str`)**.  
> - **Proyección Compacta (`Proy. Compact B/char`)** (`sais_compact_bpc`): Proyección teórica partiendo de una base de nivel 0 de $1\text{ B/char}$ (`s`) $+ 4\text{ B/char}$ (`SA`) $+ 0.125\text{ B/char}$ (`t` en $1\text{ bit/char}$) $= 5.125\text{ B/char}$, dando un rango de **`5.18–6.10 B/char` en $n = 2{,}000{,}000$** (y `5.18–6.49 B/char` considerando desde $n = 2{,}000$). Adicionalmente, el binario consulta el RSS del proceso vía `GetProcessMemoryInfo` (`#ifdef _WIN32`, devuelve `0` en Linux).

### 4.1. Escalabilidad en Tamaños de Millones de Caracteres ($n = 1{,}000{,}000$ y $n = 2{,}000{,}000$)

Datos extraídos de [`results/benchmark_metrics.csv`](results/benchmark_metrics.csv):

| Familia de Cadena | Tamaño ($n$) | Tiempo DC3 (ms) | Tiempo SA-IS (ms) | Speedup (`DC3 / SA-IS`) | Razón Reducción ($\alpha_{\text{DC3}}$ / $\alpha_{\text{SA-IS}}$) | Profundidad Recursión (DC3 / SA-IS) | Memoria Analítica 32b DC3 / SA-IS (Proy. Compact) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **`random_ascii`** ($\lvert\Sigma\rvert=64$) | `1,000,000` | `59.53` | `56.00` | `1.06x` | `0.667` / `0.331` | `1` / `1` | `19.63` / `14.36` (`6.20`) |
| **`random_ascii`** ($\lvert\Sigma\rvert=64$) | `2,000,000` | `147.12` | `155.18` | `0.95x` | `0.667` / `0.331` | `1` / `1` | `19.19` / `14.26` (`6.10`) |
| **`dna_genome`** ($\lvert\Sigma\rvert=4$) | `1,000,000` | `82.72` | **`44.87`** | **`1.84x`** | `0.667` / `0.355` | `4` / `4` | `23.21` / `14.46` (`5.29`) |
| **`dna_genome`** ($\lvert\Sigma\rvert=4$) | `2,000,000` | `194.85` | **`114.19`** | **`1.71x`** | `0.667` / `0.355` | `4` / `4` | `23.21` / `14.44` (`5.29`) |
| **`source_code`** (C/C++) | `1,000,000` | `79.35` | **`36.98`** | **`2.15x`** | `0.667` / `0.337` | `4` / `4` | `23.21` / `14.13` (`5.27`) |
| **`source_code`** (C/C++) | `2,000,000` | `189.16` | **`118.42`** | **`1.60x`** | `0.667` / `0.337` | `4` / `4` | `23.21` / `14.11` (`5.26`) |
| **`periodic_str`** (Periódica) | `1,000,000` | `46.04` | **`25.52`** | **`1.80x`** | `0.667` / `0.308` | `12` / `3` | `23.96` / `13.64` (`5.18`) |
| **`periodic_str`** (Periódica) | `2,000,000` | `95.76` | **`72.66`** | **`1.32x`** | `0.667` / `0.308` | `13` / `3` | `23.98` / `13.64` (`5.18`) |

> **Observaciones clave del escalamiento:**  
> 1. **Baseline Ingenuo (`Naive SA`):** En `periodic_str` con apenas $n = 10{,}000$, `Naive SA` tarda **`183.66 ms`**, mientras que DC3 toma `0.65 ms` ($282\times$ más rápido) y SA-IS toma **`0.34 ms`** ($540\times$ más rápido).  
> 2. **Efecto de la profundidad recursiva (`Depth = 4` vs. `Depth = 1`):** En `dna_genome` y `source_code` ($n \ge 500{,}000$), alcanzar `Depth = 4` implica **5 niveles totales ($k = 0..4$)**, donde DC3 ejecuta **20 llamadas a `radixPass` y 5 fases de `merge`** ($\sum_{k=0}^4 (2/3)^k \approx 2.605n$), mientras que SA-IS acumula $\approx 1.542n$ en `dna_genome` ($\alpha \approx 0.355$) y $\approx 1.502n$ en `source_code` ($\alpha \approx 0.337$). En `periodic_str`, la profundidad de DC3 obedece exactamente a $\lfloor \log_3 n \rfloor = 6, 8, 9, 10, 11, 12, 13$ para $n = 2\text{K}\dots 2\text{M}$ ($3^{13} \approx 1.59\text{M}$).  
> 3. **Transición de Caché L3 (16 MB) a DRAM:** El conteo de operaciones escala linealmente $\mathcal{O}(n)$, mientras que el incremento de tiempo físico entre $n = 1\text{M}$ y $n = 2\text{M}$ ($2.08\times\text{--}3.20\times$) se debe a que el espacio de trabajo ($\sim 28\text{--}48\text{ MB}$) rebasa los $16\text{ MB}$ de caché L3 del procesador hacia la DRAM.

---

### 4.2. Rendimiento sobre Archivos de Corpus de $1\text{ MB}$ en `data/`

Datos extraídos de [`results/corpus_benchmark.csv`](results/corpus_benchmark.csv) (evaluando `corpus_source_code.txt` como insumo representativo para herramientas de preprocesamiento léxico y analizadores sintácticos $LR(1)$):

| Archivo de Corpus | Tamaño ($n$) | Tiempo DC3 (ms) | Tiempo SA-IS (ms) | Speedup (`DC3 / SA-IS`) | Razón Reducción ($\alpha_{\text{SA-IS}}$) | Profundidad Recursión (DC3 / SA-IS) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| [`data/corpus_random.txt`](data/corpus_random.txt) | `1,000,000` | `64.61` | `60.75` | `1.06x` | `0.331` | `1` / `1` |
| [`data/corpus_dna.fasta`](data/corpus_dna.fasta) | `1,000,000` | `84.72` | **`44.65`** | **`1.90x`** | `0.347` | `4` / `3` |
| [`data/corpus_source_code.txt`](data/corpus_source_code.txt) | `1,000,000` | `85.92` | **`38.19`** | **`2.25x`** | **`0.304`** | `5` / `5` |
| [`data/corpus_periodic.txt`](data/corpus_periodic.txt) | `1,000,000` | `57.01` | **`34.74`** | **`1.64x`** | `0.381` | `12` / `8` |

---

### 4.3. Perfilado de Localidad de Caché L1D (32 KB) y L2 (256 KB) ($n = 200{,}000$)

Datos extraídos de [`results/cache_metrics.csv`](results/cache_metrics.csv) (en SA-IS el barrido sobre `SA[i]` es secuencial, mientras que las lecturas `s[SA[i]-1]` y `t[SA[i]-1]` representan su $\sim 36.2\%$ de accesos indirectos, frente al $52.0\%\text{--}57.2\%$ de DC3):

| Familia ($n = 200{,}000$) | % Accesos Indirectos (DC3 vs. SA-IS) | Tasa de Fallos L1 (DC3 vs. SA-IS) | Fallos L1 por Carácter (DC3 vs. SA-IS) | Reducción de Fallos L1 en SA-IS |
| :--- | :---: | :---: | :---: | :---: |
| **`random_ascii`** | `53.24%` vs. **`35.72%`** | `33.28%` vs. **`24.85%`** | `18.40` vs. **`14.86`** | **`1.24x` menos fallos** |
| **`dna_genome`** | `54.92%` vs. **`36.54%`** | `27.76%` vs. **`17.31%`** | `23.26` vs. **`11.85`** | **`1.96x` menos fallos** |
| **`source_code`** | `55.47%` vs. **`36.22%`** | `29.21%` vs. **`19.05%`** | `24.23` vs. **`12.57`** | **`1.93x` menos fallos** |
| **`periodic_str`** | `57.18%` vs. **`36.31%`** | `21.67%` vs. **`16.70%`** | `19.74` vs. **`10.24`** | **`1.93x` menos fallos** |

---

## 5. Visualizaciones Generadas (`results/`)

1. **[`results/fig1_time_scaling.png`](results/fig1_time_scaling.png)**: Curvas de escalabilidad temporal $T(n)$ en las 4 familias de cadenas ($n = 2{,}000$ a $2{,}000{,}000$).
2. **[`results/fig2_speedup_by_family.png`](results/fig2_speedup_by_family.png)**: Evolución del factor de aceleración $\text{Speedup} = T_{\text{DC3}} / T_{\text{SA-IS}}$ por familia.
3. **[`results/fig3_memory_and_recursion.png`](results/fig3_memory_and_recursion.png)**: Comparativa de memoria analítica (`B/char`), razón de reducción $\alpha$ y profundidad máxima de recursión en $n = 2{,}000{,}000$.
4. **[`results/fig4_cache_and_corpus.png`](results/fig4_cache_and_corpus.png)**: Fallos de caché L1/L2 por carácter y tiempos de ejecución sobre los 4 archivos de corpus de $1\text{ MB}$ en `data/`.

---

## 6. Documentación Académica y Defensa del Proyecto (`docs/`)

- 📄 **[Informe Técnico Final (`docs/INFORME_TECNICO_FINAL.md`)](docs/INFORME_TECNICO_FINAL.md)**: Artículo académico completo (estilo IEEE/ACM) con marco teórico, aclaraciones metodológicas de memoria, tablas experimentales hasta $n = 2{,}000{,}000$ y figuras integradas.
- 🎤 **[Guía de Exposición y Defensa Oral (`docs/GUIA_EXPOSICION_Y_DEFENSA.md`)](docs/GUIA_EXPOSICION_Y_DEFENSA.md)**: Estructura sugerida de diapositivas minuto a minuto, los 5 puntos técnicos clave del código C++17 y un banco de 8 preguntas difíciles de jurado con sus respuestas exactas.
- 📚 **[Fundamentos de los Papers Base (`docs/FUNDAMENTOS_PAPERS_BASE.md`)](docs/FUNDAMENTOS_PAPERS_BASE.md)**: Desglose teórico paso a paso de Kärkkäinen & Sanders (2003) y Nong, Zhang & Chan (2009).
- 🗺️ **[Propuesta y Ruta de Aprendizaje (`docs/PROPUESTA_Y_RUTA_APRENDIZAJE.md`)](docs/PROPUESTA_Y_RUTA_APRENDIZAJE.md)**: Planificación pedagógica original en 5 etapas.
