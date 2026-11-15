# Propuesta de Investigación y Ruta de Desarrollo Paso a Paso: DC3 vs. SAIS
**Curso:** CS3014 Estructuras de Datos Avanzadas - Proyecto de Curso I (UTEC 2026-2)
**Estado actual:** Inicio desde cero sobre la estructura base del repositorio `ProyectoEDA`.

---

## 1. Contenido Íntegro de la Propuesta del Proyecto

### Resumen del Proyecto
El análisis de grandes volúmenes de texto y la localización rápida de patrones son problemas centrales en la computación moderna, con aplicaciones directas desde la bioinformática hasta la construcción de compiladores. Los Arreglos de Sufijos (*Suffix Arrays*) destacan como una alternativa más compacta y eficiente frente a los Árboles de Sufijos. Este proyecto plantea implementar en C++ y comparar experimentalmente dos de los algoritmos lineales $\mathcal{O}(n)$ más importantes para construir arreglos de sufijos: el algoritmo **DC3** (*Difference Cover modulo 3* / *Skew*) y el algoritmo **SAIS** (*Suffix Array Induced Sorting*). La evaluación medirá el tiempo de ejecución, el uso de memoria RAM y el aprovechamiento de la memoria caché, orientando el estudio hacia su utilidad práctica en analizadores léxicos rápidos.

### 1.1. Introducción y Motivación
* **El problema de indexación:** Un Arreglo de Sufijos de una cadena $S$ de longitud $n$ es un arreglo de enteros que almacena las posiciones iniciales de todos los sufijos de $S$ ordenados lexicográficamente. Durante mucho tiempo su construcción fue un cuello de botella hasta el surgimiento de algoritmos directos en tiempo lineal $\mathcal{O}(n)$.
* **Contraste teórico vs. práctico:** Aunque ambos algoritmos garantizan tiempo lineal $\mathcal{O}(n)$, utilizan filosofías muy distintas. DC3 emplea una estrategia de "divide y vencerás" apoyada en aritmética modular (módulo 3), mientras que SAIS utiliza ordenamiento inducido (*Induced Sorting*), logrando en la práctica mayor velocidad y un menor consumo de memoria.
* **Aplicación real:** Comprender estas diferencias prácticas es clave para optimizar la lectura de tokens en herramientas reales, como la construcción de un parser $LR(1)$, además de servir como base para la transformada de Burrows-Wheeler (BWT), el FM-index y la compresión de datos.

### 1.2. Conceptos Preliminares y Trabajo Relacionado
* **Definiciones de cadenas:** Dado un alfabeto ordenado $\Sigma$, una cadena $S$ de tamaño $n$ es una secuencia $S = S[0]S[1]\dots S[n-1]$ con $S[i] \in \Sigma$. Un prefijo tiene la forma $S[0\dots i]$ y un sufijo es la subcadena $S[i\dots n-1]$. Por convención, se agrega al final un carácter centinela $\$$ que es lexicográficamente menor que cualquier elemento de $\Sigma$.
* **Clasificación de sufijos (Base de SAIS):**
  * **Tipo S (*Smaller*):** Un sufijo $S[i\dots n-1]$ es de tipo S si es lexicográficamente menor que el sufijo siguiente $S[i+1\dots n-1]$.
  * **Tipo L (*Larger*):** Un sufijo $S[i\dots n-1]$ es de tipo L si es lexicográficamente mayor que $S[i+1\dots n-1]$.
  * **Caracteres LMS (*Leftmost S-type*):** Son aquellos caracteres de tipo S cuyo carácter inmediato a la izquierda es de tipo L. Ordenar únicamente este subconjunto reducido permite inducir el orden del resto de sufijos.
* **Artículos base del estado del arte:**
  * **DC3 (*Skew Algorithm*):** Propuesto por Kärkkäinen y Sanders (2003) en *"Simple Linear Work Suffix Array Construction"*. Separa los sufijos en dos grupos según su índice módulo 3 ($i \bmod 3 \in \{1, 2\}$ frente a $i \bmod 3 = 0$), resuelve recursivamente el primer grupo y fusiona ambos resultados.
  * **SAIS:** Propuesto por Nong, Zhang y Chan (2009) en *"Linear Suffix Array Construction by Almost Pure Induced-Sorting"*. Perfecciona el ordenamiento inducido reduciendo el subproblema recursivo a lo sumo a la mitad de la cadena original ($\le \lfloor n/2 \rfloor$), logrando complejidad espacial lineal con constantes muy bajas.

### 1.3. Planteamiento, Objetivos y Restricciones Técnicas
* **Objetivo General:** Estudiar, implementar y comparar empíricamente DC3 y SAIS frente a diversas distribuciones y longitudes de cadenas.
* **Objetivos Específicos:**
  1. Implementar DC3 siguiendo la especificación de Kärkkäinen y Sanders.
  2. Implementar SAIS siguiendo la estrategia de *Induced Sorting* de Nong et al..
  3. Ejecutar pruebas de estrés midiendo el tiempo real de construcción (*wall-clock time*) en función de $n$.
  4. Analizar el consumo de memoria RAM y el comportamiento de la memoria caché (*cache misses*) durante la ejecución.
* **Justificación de C++ Nativo:** Ambas implementaciones se desarrollarán nativamente en C++ para tener control estricto de la memoria y permitir un perfilado (*profiling*) real del hardware sin las penalizaciones de una máquina virtual.

### 1.4. Entregables Esperados y Obstáculos Previstos
* **Entregables comprometidos:**
  1. Código fuente completo y comentado de ambos algoritmos.
  2. Scripts para reproducir los experimentos.
  3. Informe técnico detallado con análisis teórico, gráficas comparativas (tiempo vs. longitud de entrada) y conclusiones sobre indexación rápida.
* **Obstáculos técnicos a superar y explicar en la exposición:**
  1. Implementar la etapa recursiva en ambos métodos manteniendo en la práctica la complejidad espacial lineal prometida en la teoría, evitando copias innecesarias de memoria.
  2. Manejar con precisión los punteros de cubetas (*buckets*) y banderas en arreglos dentro de SAIS para evitar accesos fuera de rango (*segfaults*) en C++ nativo.

---

## 2. Mapeo del Cronograma Original (Semanas 9 a 15) sobre el Repositorio `ProyectoEDA`

Aunque nos encontramos cronológicamente en la Semana 14, el desarrollo se ejecutará recorriendo **en estricto orden pedagógico las 5 fases planeadas** para garantizar el dominio total del código de cara a la exposición:

### Etapa 1 (Equivalente a Semana 9): Estudio Teórico, Configuración y Base de Validación
* **Meta de aprendizaje:** Comprender cómo se transforma una cadena `std::string` en un arreglo de enteros con centinela y cómo verificar si un Suffix Array es correcto.
* **Archivos a crear en `ProyectoEDA`:**
  * `CMakeLists.txt` o script de compilación limpia.
  * `include/naive_sa.hpp`: Implementación ingenua por comparación directa para validar correctitud en cadenas pequeñas.
  * `tests/test_runner.cpp`: Primeras pruebas con las cadenas ejemplo de los papers (`mississippi` y `mmiississippii`).

### Etapa 2 (Equivalente a Semanas 10 y 11): Implementación Paso a Paso de DC3
* **Meta de aprendizaje:** Dominar el ordenamiento por conteo (*Radix Pass*), por qué se añade un elemento *dummy* cuando $n \bmod 3 = 1$, cómo funciona el nombramiento lexicográfico de tripletas y por qué la fase de *Merge* puede comparar dos sufijos en tiempo $\mathcal{O}(1)$.
* **Archivos a crear en `ProyectoEDA`:**
  * `include/dc3.hpp` y `src/dc3.cpp` (construidos incrementalmente: primero `radixPass`, luego el muestreo $\bmod 3 \neq 0$, luego el ordenamiento de $\bmod 3 = 0$, y finalmente el `merge`).

### Etapa 3 (Equivalente a Semanas 12 y 13): Implementación Paso a Paso de SAIS
* **Meta de aprendizaje:** Dominar el barrido de derecha a izquierda para clasificar tipos L y S, el concepto de subcadenas LMS, la partición de cada cubeta (*bucket*) en zona L (izquierda) y zona S (derecha), y los 3 barridos del ordenamiento inducido.
* **Archivos a crear en `ProyectoEDA`:**
  * `include/sais.hpp` y `src/sais.cpp` (construidos incrementalmente y validados contra `naive_sa` y `dc3`).

### Etapa 4 (Equivalente a Semana 14): Experimentación y Benchmarks
* **Meta de aprendizaje:** Aprender a medir *wall-clock time*, consumo pico de memoria RAM en el *heap* y fallos de caché (*cache misses*) sobre texto aleatorio, genomas y código fuente.
* **Archivos a crear en `ProyectoEDA`:**
  * `bench/benchmark.cpp`, poblado de `data/`, exportación de CSV en `results/` y automatización en `scripts/`.

### Etapa 5 (Equivalente a Semana 15): Análisis de Resultados y Preparación de Exposición
* **Meta de aprendizaje:** Conectar las métricas obtenidas (razón de reducción de $2/3$ en DC3 vs $\le 1/2$ o $\approx 1/3$ en SAIS) con las gráficas experimentales y redactar el informe final en `docs/`.