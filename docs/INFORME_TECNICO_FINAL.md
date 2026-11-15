# Construcción en Tiempo Lineal de Arreglos de Sufijos mediante los Algoritmos DC3 y SA-IS: Análisis Teórico, Implementación en C++ y Evaluación Experimental de Memoria y Localidad de Caché

**Curso:** CS3014 Estructuras de Datos Avanzadas · Proyecto de Curso I (UTEC 2026-2)  
**Repositorio:** `ProyectoEDA`

---

## Resumen (*Abstract*)

Los **Arreglos de Sufijos** (*Suffix Arrays*, $SA$) constituyen la estructura de indexación de texto completo más utilizada en bioinformática, compresión de datos sin pérdida (transformada de Burrows-Wheeler y FM-index) y preprocesamiento léxico rápido en compiladores. Aunque durante más de una década su construcción directa requirió tiempo superlineal $\mathcal{O}(n \log n)$, el desarrollo de algoritmos puramente lineales $\mathcal{O}(n)$ demostró que es posible prescindir de la costosa construcción intermedia de un Árbol de Sufijos. En este informe se presenta el diseño, implementación nativa en C++17 y evaluación experimental exhaustiva de los dos algoritmos lineales más influyentes del estado del arte: **DC3 / Skew** (Kärkkäinen & Sanders, 2003), fundamentado en cubiertas de diferencias módulo 3 y ordenamiento por conteo (*Radix Sort*), y **SA-IS** (Nong, Zhang & Chan, 2009), fundamentado en clasificación de sufijos $L/S$, muestreo de subcadenas *Leftmost S-type* (LMS) y ordenamiento inducido (*Induced Sorting*). Evaluamos ambos métodos frente a un generador ingenuo $\mathcal{O}(n^2 \log n)$ sobre cuatro familias de datos (texto aleatorio uniforme, genomas de ADN, código fuente C++ orientado a analizadores léxicos $LR(1)$ y cadenas periódicas de Fibonacci) escalando hasta $n = 500{,}000$ caracteres, e instrumentamos un simulador de jerarquía de caché L1D (32 KB, 8 vías, líneas de 64 B) y L2 (256 KB). Los resultados experimentales demuestran que SA-IS supera consistentemente a DC3 logrando un *speedup* de **$1.16\times$ a $2.51\times$** en tiempo de ejecución, una reducción del **$38\%\text{--}42\%$ en memoria RAM con enteros de 32 bits** (y hasta **$4.4\times$ menos memoria** en el modelo compacto de $5.20\text{--}6.38\text{ B/carácter}$), y hasta **$1.96\times$ menos fallos de caché L1 por carácter**, explicando desde la arquitectura de computadores cómo la razón de reducción adaptativa ($\approx 0.265\text{--}0.355$ frente al $0.667$ fijo de DC3) y los barridos secuenciales de inducción determinan el rendimiento práctico.

**Palabras Clave:** Arreglo de Sufijos (*Suffix Array*), Tiempo Lineal $\mathcal{O}(n)$, Algoritmo DC3 / Skew, Algoritmo SA-IS, Ordenamiento Inducido (*Induced Sorting*), Subcadenas LMS, Localidad de Caché, Analizadores Léxicos.

---

## 1. Introducción y Motivación

### 1.1. El Problema de Indexación de Texto Completo
Sea $\Sigma$ un alfabeto totalmente ordenado y $S = S[0]S[1]\dots S[n-1]$ una cadena de longitud $n$ sobre $\Sigma$, a la cual se concatena conceptualmente un carácter centinela único $S[n] = \$ \notin \Sigma$ tal que $\$ < c$ para todo $c \in \Sigma$. Denotamos por $S_i = S[i\dots n]$ al sufijo que inicia en la posición $i \in \{0, 1, \dots, n-1\}$. El **Arreglo de Sufijos** $SA[0\dots n-1]$ es la permutación única de $\{0, 1, \dots, n-1\}$ que satisface:

$$S_{SA[0]} < S_{SA[1]} < \dots < S_{SA[n-1]}$$

A diferencia de los Árboles de Sufijos (*Suffix Trees*), que si bien pueden construirse en tiempo $\mathcal{O}(n)$ mediante los algoritmos de Weiner, McCreight, Ukkonen o Farach consumen entre $10n$ y $20n$ bytes de memoria debido a los punteros de nodos y aristas, el Arreglo de Sufijos requiere únicamente $4n$ bytes (un entero de 32 bits por carácter).

### 1.2. De $\mathcal{O}(n^2 \log n)$ a $\mathcal{O}(n)$ y su Aplicación en Analizadores Léxicos
La construcción ingenua de $SA$ ordenando los $n$ sufijos mediante un algoritmo basado en comparaciones (`std::sort`) realiza $\mathcal{O}(n \log n)$ comparaciones de cadenas, cada una de costo $\mathcal{O}(n)$ en el peor caso, degradándose a $\mathcal{O}(n^2 \log n)$ cuando existen prefijos comunes largos (como ocurre en código fuente con bloques repetidos o secuencias genómicas). En el contexto de compiladores modernos, indexar rápidamente archivos de código fuente para detección de tokens repetidos, autocompletado de símbolos y preprocesamiento en parsers $LR(1)$ exige tanto **complejidad temporal lineal $\mathcal{O}(n)$** como una **huella mínima en memoria caché** para no desplazar las tablas de análisis sintáctico.

---

## 2. Marco Teórico y Algoritmos Implementados

### 2.1. Algoritmo DC3 / Skew (Kärkkäinen & Sanders, 2003)

El algoritmo DC3 (*Difference Cover modulo 3*) explota el hecho de que el conjunto $D = \{1, 2\}$ forma una **cubierta de diferencias módulo 3**: para cualesquiera dos posiciones $i, j$, siempre existe un desplazamiento $\Delta \in \{0, 1, 2\}$ tal que tanto $(i + \Delta) \bmod 3 \in \{1, 2\}$ como $(j + \Delta) \bmod 3 \in \{1, 2\}$.

```mermaid
flowchart TD
    A["Entrada s[0..n-1] + padding s[n]=s[n+1]=s[n+2]=0"] --> B["Paso 1: Muestrear posiciones i mod 3 != 0 (n02 = n0 + n2)"]
    B --> C["Ordenar tripletas (s[i], s[i+1], s[i+2]) con 3 pasadas de radixPass"]
    C --> D["Asignar nombres lexicográficos en s12 (Mitad izq: mod 3 = 1 | Mitad der: mod 3 = 2)"]
    D --> E{"¿Nombres únicos? (name == n02)"}
    E -- "No (name < n02)" --> F["Llamada recursiva suffixArray(s12, SA12, n02, name) + invertir SA12 en s12"]
    E -- "Sí (Caso base)" --> G["Construcción directa SA12[s12[i] - 1] = i"]
    F --> H["Paso 2: Extraer s0 desde SA12 y ordenar SA0 en 1 pasada de radixPass"]
    G --> H
    H --> I["Paso 3: Merge lineal O(n) entre SA12 y SA0 usando leq() en O(1)"]
```

1. **Tamaños por residuo y el sufijo *dummy* cuando $n \bmod 3 = 1$:**
   Definimos $n_0 = \lfloor(n+2)/3\rfloor$, $n_1 = \lfloor(n+1)/3\rfloor$, $n_2 = \lfloor n/3 \rfloor$ y $n_{02} = n_0 + n_2$. Cuando $n \bmod 3 = 1$, hay un sufijo más con residuo $0$ que con residuo $1$ ($n_0 - n_1 = 1$). Como el último sufijo múltiplo de 3 ($i = n - 1$) necesita apoyarse en su sucesor $S_n$ (con residuo $n \bmod 3 = 1$) para poder ser ordenado en el Paso 2, el bucle de muestreo recorre hasta $i < n + (n_0 - n_1)$, insertando la tripleta ficticia $(0, 0, 0)$ en $i = n$.
2. **Ordenamiento de tripletas y mapeo en `s12`:**
   Las $n_{02}$ tripletas $(s[i], s[i+1], s[i+2])$ se ordenan en $\mathcal{O}(n + K)$ mediante tres llamadas estables a `radixPass` con punteros desplazados `s + 2`, `s + 1` y `s`. A continuación, se asignan nombres lexicográficos $name \in [1, n_{02}]$:
   * Si $\text{SA12}[i] \bmod 3 == 1$, el nombre se escribe en la mitad izquierda: `s12[SA12[i] / 3] = name`.
   * Si $\text{SA12}[i] \bmod 3 == 2$, el nombre se escribe en la mitad derecha: `s12[SA12[i] / 3 + n0] = name`.
3. **Resolución de `SA12` y arreglo inverso $\overline{SA}^{12}$:**
   Si $name < n_{02}$, hubo tripletas idénticas y se invoca recursivamente `suffixArray(s12, SA12, n02, name)`, invirtiendo luego `SA12` sobre `s12` mediante `s12[SA12[i]] = i + 1`. Si $name == n_{02}$, los nombres en `s12` ya son los rangos inversos únicos y `SA12` se genera directamente con `SA12[s12[i] - 1] = i`.
4. **Ordenamiento de `SA0` y Fusión (*Merge*) en $\mathcal{O}(1)$ por comparación:**
   Cada sufijo $S_{3k}$ ($i \bmod 3 = 0$) se expresa como $(s[3k], S_{3k+1})$. Como $S_{3k+1}$ vive en el índice $k < n_0$ de `s12`, basta filtrar `if (SA12[i] < n0) s0[j++] = 3 * SA12[i]` y ejecutar **una sola pasada** `radixPass(s0, SA0, s, n0, K)`. Finalmente, el `merge` inicializa el cursor $t = n_0 - n_1$ (descartando el *dummy* en `SA12[0]` si $n \bmod 3 = 1$) y compara $S_i \in SA^{12}$ contra $S_j \in SA^0$ en tiempo constante $\mathcal{O}(1)$:
   * **Caso $i \bmod 3 = 1$ (`SA12[t] < n0`):** Avanza $1$ carácter y compara pares `leq(s[i], s12[SA12[t] + n0], s[j], s12[j / 3])`, ya que tanto $i+1 \equiv 2$ como $j+1 \equiv 1 \pmod 3$ están en `s12`.
   * **Caso $i \bmod 3 = 2$ (`SA12[t] >= n0`):** Avanza $2$ caracteres (porque $i+1 \equiv 0 \pmod 3$ no está en `s12`) y compara tripletas `leq(s[i], s[i+1], s12[SA12[t] - n0 + 1], s[j], s[j+1], s12[j / 3 + n0])`, donde $i+2 \equiv 1$ y $j+2 \equiv 2 \pmod 3$.

---

### 2.2. Algoritmo SA-IS (Nong, Zhang & Chan, 2009)

SA-IS reemplaza la división aritmética fija por una partición estructural basada en las transiciones locales de la cadena $s[0\dots n]$ terminada en el centinela $s[n] = 0$:

```mermaid
flowchart TD
    A["Entrada s[0..n] con centinela s[n] = 0"] --> B["Clasificación L (0) y S (1) de derecha a izquierda en O(n)"]
    B --> C["Sembrar posiciones LMS (t[i]==S && t[i-1]==L) al final de sus cubetas S"]
    C --> D["1.ª Inducción (Ordenar subcadenas LMS): induceSAl (izq->der) + induceSAs (der->izq)"]
    D --> E["Compactar n1 LMS en SA[0..n1-1] y nombrar en SA[n1 + pos/2] -> s1 en SA[n+1-n1..n]"]
    E --> F{"¿Nombres únicos? (name == n1)"}
    F -- "No (name < n1)" --> G["Llamada recursiva sais_core(s1, SA1, n1 - 1, name - 1)"]
    F -- "Sí (Caso base)" --> H["Construcción directa SA1[s1[i]] = i"]
    G --> I["Traducir SA1 a índices de s, sembrar de derecha a izquierda en colas S"]
    H --> I
    I --> J["2.ª Inducción (SA completo): induceSAl (izq->der) + induceSAs (der->izq)"]
```

1. **Clasificación $L/S$ y Subcadenas LMS (Lema 2.1):**
   Un barrido de $i = n - 1$ hacia $0$ con $t[n] = \text{S}$ asigna $t[i] = \text{S}$ si $s[i] < s[i+1]$, $t[i] = \text{L}$ si $s[i] > s[i+1]$, y $t[i] = t[i+1]$ si empatan. Una posición $i > 0$ es **LMS** si $t[i] == \text{S} \land t[i-1] == \text{L}$. Como $i = 0$ nunca es LMS y jamás pueden existir dos posiciones LMS adyacentes, el número de elementos LMS cumple:
   $$n_1 \le \left\lfloor \frac{n + 1}{2} \right\rfloor$$
   En textos con caracteres equiprobables (Teorema 3.2), la longitud esperada entre dos LMS es $3$, reduciendo el problema en promedio a $\mathbb{E}[n_1 / n] \le 1/3$.
2. **Estructura de Cubetas y Motor de Inducción (`induceSAl` e `induceSAs`):**
   Dentro de la cubeta de cualquier carácter $c$, todo sufijo tipo L es lexicográficamente menor que todo sufijo tipo S (pues el primer carácter distinto tras la racha de $c$'s es $< c$ en un sufijo L y $> c$ en un sufijo S). Por ello, `induceSAl` barre `SA` de izquierda a derecha ($0 \to n$) insertando $j = \text{SA}[i] - 1$ (si $t[j] == \text{L}$) en la cabeza de su cubeta (`bkt[s[j]]++`), e `induceSAs` barre de derecha a izquierda ($n \to 0$) insertando $j = \text{SA}[i] - 1$ (si $t[j] == \text{S}$) en la cola de su cubeta (`bkt[s[j]]--`).
3. **Nombramiento con el truco `pos / 2` y reutilización in-place de `SA`:**
   Tras la primera inducción, los $n_1$ índices LMS se compactan en `SA[0..n1-1]`. Se comparan subcadenas LMS adyacentes verificando tanto `s[pos + d]` como `t[pos + d]` hasta el siguiente LMS inclusive. Como dos posiciones LMS nunca son consecutivas, dividir su coordenada entre 2 (`pos / 2`) permite escribir su nombre en `SA[n1 + pos / 2]` en orden posicional sin colisiones y sin alojar arreglos adicionales, compactando luego `s1` en `SA[n + 1 - n1 .. n]`.

---

### 2.3. Comparativa Teórica de Recurrencias y Constantes Ocultas

| Propiedad Teórica | Algoritmo DC3 / Skew (2003) | Algoritmo SA-IS (2009) |
| :--- | :--- | :--- |
| **Recurrencia Temporal** | $T(n) = T(\lceil 2n/3 \rceil) + \mathcal{O}(n)$ | $T(n) \le T(\lfloor n/2 \rfloor) + \mathcal{O}(n)$ ($\approx T(n/3) + \mathcal{O}(n)$ medio) |
| **Factor de Serie Geométrica $\sum \alpha^k$** | $\frac{1}{1 - 2/3} = \mathbf{3.0n}$ (fijo en toda entrada) | $\le \frac{1}{1 - 1/2} = 2.0n$ peor caso; $\approx \frac{1}{1 - 1/3} = \mathbf{1.5n}$ típico |
| **Razón de Reducción ($\alpha$)** | Determinista: $\frac{2}{3} \approx \mathbf{0.667}$ | Adaptativa: $\le 0.500$ (observada: $\mathbf{0.265\text{--}0.381}$) |
| **Reservas Dinámicas por Nivel** | 4 arreglos nuevos (`s12`, `SA12`, `s0`, `SA0`) + `c` | 2 arreglos (`t`, `bkt`); **`s1` y `SA1` reutilizan `SA` in-place** |
| **Patrón de Acceso a Memoria** | 4 pasadas de *Radix Sort* con saltos indirectos `r[a[i]]` | Barridos secuenciales (`i++` / `i--`) y punteros contiguos `bkt[c]` |

---

## 3. Arquitectura de Software y Metodología Experimental

El proyecto fue desarrollado desde cero en **C++17 nativo** (compilado con `g++ 14.2.0` bajo banderas `-Wall -Wextra -Wpedantic -O3`) sin dependencias externas:
* **Cabeceras e Implementaciones (`include/` y `src/`):**
  * [`include/naive_sa.hpp`](../include/naive_sa.hpp): Generador de referencia [`eda::build_naive_sa`](../include/naive_sa.hpp#L29-L62) con `std::string_view` ($\mathcal{O}(n)$ en memoria) y auditor independiente [`eda::verify_suffix_array`](../include/naive_sa.hpp#L93-L133).
  * [`include/dc3.hpp`](../include/dc3.hpp) y [`src/dc3.cpp`](../src/dc3.cpp): Implementación de `leq`, `radixPass`, `dc3Phase1`, `suffixArray`, `build_dc3_sa` y rastreador de memoria `DC3Tracker`.
  * [`include/sais.hpp`](../include/sais.hpp) y [`src/sais.cpp`](../src/sais.cpp): Implementación de `isLMS`, `lmsSubstringsAreEqual`, `classifyLS`, `getBuckets`, `induceSAl`, `induceSAs`, `sais_core`, `build_sais_sa` y rastreador `SAISTracker`.
* **Suite de Correctitud ([`tests/test_runner.cpp`](../tests/test_runner.cpp)):** Verifica todas las trazas intermedias de los papers (`"mississippi"`, `"mmiississippii"`, `"mmiissiissiippii"`, `"bananas"`) y realiza validación cruzada triple (`SA-IS == DC3 == Naive SA`) sobre 25 escenarios borde.
* **Motor de Benchmarks y Simulador de Caché ([`bench/benchmark.cpp`](../bench/benchmark.cpp)):**
  * Mide tiempo de pared con `std::chrono::steady_clock` (1 ronda de *warm-up* + 5 repeticiones reportando mediana y media).
  * Modela en [`CacheHierarchySimulator`](../bench/benchmark.cpp#L42-L129) una caché de datos **L1D de 32 KB** (64 conjuntos $\times$ 8 vías, líneas de 64 bytes = 16 `int`s por línea) y **L2 de 256 KB** (512 conjuntos $\times$ 8 vías) con reemplazo LRU exacto.

---

## 4. Resultados Experimentales y Discusión Crítica

### 4.1. Escalamiento del Tiempo de Construcción vs. Tamaño de Entrada $n$

La Tabla 1 resume los tiempos medianos de construcción (en milisegundos) extraídos de [`results/benchmark_metrics.csv`](../results/benchmark_metrics.csv) para las 4 familias sintéticas desde $n = 2{,}000$ hasta $n = 500{,}000$:

| Familia | $n$ | Naive (ms) | DC3 (ms) | SA-IS (ms) | Speedup (DC3 / SA-IS) | Profundidad (DC3 / SA-IS) | Razón Reducción (DC3 / SA-IS) |
| :--- | ---: | ---: | ---: | ---: | :---: | :---: | :---: |
| **`random_ascii`** | $10{,}000$ | $0.99$ | $0.41$ | $0.39$ | $1.04\times$ | $1 / 1$ | $0.667 / 0.329$ |
| **`random_ascii`** | $100{,}000$ | --- | $4.35$ | $3.89$ | $1.12\times$ | $1 / 1$ | $0.667 / 0.330$ |
| **`random_ascii`** | $500{,}000$ | --- | $25.59$ | $21.98$ | $\mathbf{1.16\times}$ | $1 / 1$ | $0.667 / 0.331$ |
| **`dna_genome`** | $10{,}000$ | $1.82$ | $0.50$ | $0.33$ | $1.53\times$ | $3 / 3$ | $0.667 / 0.356$ |
| **`dna_genome`** | $100{,}000$ | --- | $5.91$ | $3.18$ | $1.86\times$ | $4 / 3$ | $0.667 / 0.355$ |
| **`dna_genome`** | $500{,}000$ | --- | $34.43$ | $17.01$ | $\mathbf{2.02\times}$ | $4 / 4$ | $0.667 / 0.355$ |
| **`source_code`** | $10{,}000$ | $1.57$ | $0.51$ | $0.31$ | $1.68\times$ | $3 / 3$ | $0.667 / 0.335$ |
| **`source_code`** | $100{,}000$ | --- | $5.56$ | $2.89$ | $1.92\times$ | $4 / 3$ | $0.667 / 0.336$ |
| **`source_code`** | $500{,}000$ | --- | $36.02$ | $16.46$ | $\mathbf{2.19\times}$ | $4 / 4$ | $0.667 / 0.337$ |
| **`periodic_str`** | $2{,}000$ | $4.36$ | $0.07$ | $0.04$ | $1.92\times$ | $6 / 3$ | $0.666 / 0.308$ |
| **`periodic_str`** | $10{,}000$ | $\mathbf{153.91}$ | $0.41$ | $0.22$ | $1.89\times$ | $8 / 3$ | $0.667 / 0.308$ |
| **`periodic_str`** | $500{,}000$ | --- | $19.84$ | $11.52$ | $\mathbf{1.72\times}$ | $\mathbf{11 / 3}$ | $0.667 / 0.308$ |

![Figura 1: Escalamiento del tiempo de construcción (DC3 vs SA-IS)](../results/fig1_time_scaling.png)

**Hallazgos clave del escalamiento temporal:**
1. **Colapso cuadrático de `Naive` ante repeticiones:** En `random_ascii` ($n = 10{,}000$), dos sufijos aleatorios difieren en sus primeros $1\text{--}2$ caracteres y `build_naive_sa` tarda solo $0.99\text{ ms}$. Sin embargo, en `periodic_str` ($n = 10{,}000$), los sufijos comparten prefijos idénticos de miles de caracteres, disparando el tiempo de `Naive` a **$153.91\text{ ms}$** ($700\times$ más lento que los $0.22\text{ ms}$ de SA-IS).
2. **Invarianza lineal $\mathcal{O}(n)$ de DC3 y SA-IS:** En las 4 gráficas de la Figura 1, tanto DC3 como SA-IS trazan rectas con pendiente constante respecto a $n$, confirmando empíricamente su cota lineal.

---

### 4.2. Análisis de *Speedup* y Desempeño en los Corpus de `data/`

![Figura 2: Speedup de SA-IS sobre DC3 por familia de cadenas](../results/fig2_speedup_by_family.png)

Al evaluar los 4 archivos de corpus reales generados en `data/` ([`results/corpus_benchmark.csv`](../results/corpus_benchmark.csv), $n \approx 250{,}000$), obtenemos:

| Archivo de Corpus (`data/`) | Tamaño $n$ | $\|\Sigma\|$ | DC3 (ms) | SA-IS (ms) | Speedup | Profundidad (D/S) | Reducción (D / S) | Memoria B/char (DC3 / SAIS / Comp) |
| :--- | ---: | :---: | ---: | ---: | :---: | :---: | :---: | :---: |
| `data/corpus_random.txt` | $250{,}000$ | $64$ | $11.67$ | $10.37$ | $1.13\times$ | $1 / 1$ | $0.667 / 0.331$ | $20.64 / 14.54 / 6.38$ |
| `data/corpus_dna.fasta` | $250{,}000$ | $4$ | $16.06$ | $8.17$ | $\mathbf{1.97\times}$ | $4 / 3$ | $0.667 / 0.348$ | $23.21 / 13.99 / 5.30$ |
| `data/corpus_source_code.txt` | $257{,}069$ | $95$ | $18.88$ | $7.54$ | $\mathbf{2.51\times}$ | $10 / 9$ | $0.667 / \mathbf{0.265}$ | $23.93 / 13.72 / 5.23$ |
| `data/corpus_periodic.txt` | $250{,}000$ | $10$ | $12.42$ | $7.48$ | $1.66\times$ | $11 / 8$ | $0.667 / 0.381$ | $23.95 / 14.46 / 5.20$ |

**¿Por qué SA-IS alcanza su máximo *speedup* ($2.51\times$) sobre el código fuente C++ (`corpus_source_code.txt`)?**
En el código fuente real, las secuencias de espacios de indentación (`"    "`), comentarios (`"// ---"`) e identificadores estructurados producen rachas monótonas largas de caracteres tipo L o tipo S antes de cambiar de dirección (`L -> S`). Como consecuencia, las subcadenas LMS tienen una longitud media de $\approx 3.77$ caracteres, comprimiendo el primer subproblema recursivo a solo el **$26.5\%$ del tamaño original ($n_1 / n = 0.265$)**, mientras que DC3 está obligado por diseño a conservar siempre el **$66.7\%$ ($n_{02} / n = 0.667$)**. Esto confirma directamente la hipótesis central de nuestra propuesta: **SA-IS es más de $2.5$ veces más rápido que DC3 para indexación léxica de código fuente**.

---

### 4.3. Consumo de Memoria RAM, Profundidad de Recursión y Razón de Reducción

![Figura 3: Consumo de Memoria RAM (B/char), Profundidad Recursiva y Razón de Reducción](../results/fig3_memory_and_recursion.png)

1. **Huella de Memoria RAM (`B/char`):**
   * En nuestra implementación con arreglos `int` de 32 bits, **DC3 consume entre $20.18$ y $23.95\text{ B/carácter}$**, ya que en cada nivel recursivo mantiene simultáneamente en el *heap* los vectores `s12`, `SA12`, `s0` y `SA0` con factor acumulativo $\sum (2/3)^k = 3$.
   * **SA-IS consume solo entre $13.64$ y $14.54\text{ B/carácter}$** en representación `int` de 32 bits, y apenas **$5.18\text{ a }6.38\text{ B/carácter}$ en el modelo compacto** (donde `s` de nivel 0 ocupa $1\text{ B/char}$, `SA` ocupa $4\text{ B/char}$ y el vector booleano `t` ocupa $1\text{ bit/char}$), reproduciendo con exactitud el valor de **$5.12\text{ B/char}$** reportado por Nong et al. (2009) en su Tabla 1.
2. **Contraste de Profundidad Recursiva en Cadenas Periódicas (`D = 11` vs `D = 3`):**
   En `periodic_str` ($n = 500{,}000$), cada nivel de DC3 solo agrupa de $3$ en $3$ caracteres ($3^1, 3^2, 3^3, \dots$) y reduce el tamaño lentamente a razón de $2/3$, requiriendo **11 niveles recursivos** para resolver los empates periódicos. En cambio, las subcadenas LMS de SA-IS capturan períodos variables completos desde el primer nivel y reducen el tamaño a razón de $0.308$, resolviendo todos los empates en solo **3 niveles recursivos**.

---

### 4.4. Localidad Espacial y Fallos de Caché L1/L2 (*Cache Misses*)

La Tabla 3 presenta las métricas obtenidas con nuestro simulador de jerarquía de caché (`CacheHierarchySimulator`, L1D de 32 KB y L2 de 256 KB con líneas de 64 B) en $n = 200{,}000$ ([`results/cache_metrics.csv`](../results/cache_metrics.csv)):

| Distribución ($n = 200{,}000$) | Accesos Indirectos DC3 | Accesos Indirectos SA-IS | Fallos L1 / char DC3 (Tasa %) | Fallos L1 / char SA-IS (Tasa %) | Ratio Fallos L1 (DC3 / SA-IS) | Ratio Fallos L2 (DC3 / SA-IS) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **`random_ascii`** | $53.24\%$ | $35.72\%$ | $18.40$ ($33.28\%$) | $14.86$ ($24.85\%$) | $1.24\times$ | $1.39\times$ |
| **`dna_genome`** | $54.92\%$ | $36.54\%$ | $23.26$ ($27.76\%$) | $11.85$ ($17.31\%$) | $\mathbf{1.96\times}$ | $\mathbf{1.67\times}$ |
| **`source_code`** | $55.47\%$ | $36.22\%$ | $24.23$ ($29.21\%$) | $12.57$ ($19.05\%$) | $\mathbf{1.93\times}$ | $\mathbf{1.67\times}$ |
| **`periodic_str`** | $57.18\%$ | $36.31\%$ | $19.74$ ($21.67\%$) | $10.24$ ($16.70\%$) | $\mathbf{1.93\times}$ | $\mathbf{1.42\times}$ |

![Figura 4: Fallos de caché L1 por carácter y desempeño en archivos de corpus](../results/fig4_cache_and_corpus.png)

**Interpretación arquitectónica:**
En una CPU moderna, una operación aritmética en registro toma $\sim 0.5\text{ ns}$ ($1\text{--}2$ ciclos), mientras que un fallo de caché L1 que acierta en L2 cuesta $\sim 4\text{ ns}$ ($\sim 12\text{ ciclos}$) y un fallo de L2 hacia L3/RAM cuesta entre $15\text{ ns}$ y $70\text{ ns}$ ($50\text{--}200\text{ ciclos}$). Como ambos algoritmos realizan únicamente operaciones enteras simples dentro de sus bucles, **el tiempo de ejecución real está gobernado por las detenciones del procesador (*memory stalls*) esperando líneas de caché**. El hecho de que DC3 sufra **$1.93\times\text{ a }1.96\times$ más fallos L1 por carácter** y **$1.67\times$ más fallos L2** en genomas y código fuente explica de manera directa y cuantitativa por qué SA-IS es **$\approx 2.0\times\text{ a }2.5\times$ más rápido en tiempo de reloj**.

---

## 5. Conclusiones y Recomendaciones Prácticas

1. **Superioridad práctica de SA-IS en tiempo, memoria y caché:** Aunque tanto DC3 como SA-IS poseen complejidad temporal asintótica $\mathcal{O}(n)$, SA-IS reduce el trabajo acumulado de la recursión de $3.0n$ a $\approx 1.36n\text{--}1.54n$, elimina las reservas dinámicas de subproblemas al almacenar `SA1` y `s1` dentro del propio arreglo `SA`, y reduce casi a la mitad los fallos de caché L1 gracias a la localidad espacial de sus barridos de inducción.
2. **Idoneidad para Analizadores Léxicos y Compiladores:** En código fuente C++ (`corpus_source_code.txt`), SA-IS alcanzó su mejor tasa de compresión estructural ($n_1 / n = 0.265$) y un *speedup* de **$2.51\times$** sobre DC3, posicionándose como el algoritmo ideal para indexación en memoria dentro de herramientas de análisis léxico y sintáctico.
3. **Cuándo sigue siendo relevante DC3:** La ventaja fundamental de DC3 reside en que su fase recursiva y su fusión pueden paralelizarse y adaptarse a modelos de **memoria externa (*External Memory*)** y **PRAM/BSP** (Teorema 2 de Kärkkäinen & Sanders, 2003), mientras que los barridos `induceSAl` e `induceSAs` de SA-IS tienen una dependencia secuencial inherente (el elemento escrito en `SA[bkt[s[j]]]` es leído iteraciones más adelante dentro del mismo barrido).

---

## 6. Referencias Bibliográficas

1. Kärkkäinen, J., & Sanders, P. (2003). *Simple linear work suffix array construction.* En **Automata, Languages and Programming (ICALP 2003)**, Lecture Notes in Computer Science, vol. 2719, pp. 943–955. Springer.
2. Nong, G., Zhang, S., & Chan, W. H. (2009). *Linear suffix array construction by almost pure induced-sorting.* En **2009 Data Compression Conference (DCC 2009)**, pp. 193–202. IEEE.
3. Manber, U., & Myers, G. (1993). *Suffix arrays: a new method for on-line string searches.* **SIAM Journal on Computing**, 22(5), 935–948.
4. Ko, P., & Aluru, S. (2003). *Space efficient linear time construction of suffix arrays.* En **Combinatorial Pattern Matching (CPM 2003)**, LNCS 2676, pp. 200–210. Springer.
