# ProyectoEDA

**Construcción en tiempo lineal de arreglos de sufijos mediante los algoritmos DC3 y SAIS**

Proyecto de curso I · CS3014 Estructuras de Datos Avanzadas · UTEC 2026-2

> Estado: 🚧 estructura inicial del repositorio. Todavía no hay código.

---

## Descripción

Un **arreglo de sufijos** (*suffix array*) de una cadena `S` de longitud `n` es un arreglo de enteros con las posiciones iniciales de todos los sufijos de `S`, ordenados lexicográficamente. Es una alternativa compacta al árbol de sufijos y sirve de base para búsqueda de patrones, la transformada de Burrows-Wheeler (BWT), el FM-index, la compresión y la bioinformática.

Este proyecto implementa en C++ y compara experimentalmente dos algoritmos que construyen el arreglo en **tiempo lineal O(n)**:

| Algoritmo | Paper base | Idea principal |
|-----------|------------|----------------|
| **DC3 / Skew** | Kärkkäinen & Sanders (2003) | Divide los sufijos según su posición módulo 3. Ordena recursivamente los de posiciones ≢ 0 (mod 3), deduce el orden de los ≡ 0 y luego fusiona ambos grupos. |
| **SAIS** | Nong, Zhang & Chan (2009) | Clasifica cada sufijo como tipo L o S, ordena recursivamente los sufijos LMS y a partir de ellos *induce* el orden del resto. El subproblema recursivo mide como máximo n/2. |

## Objetivos

1. Implementar **DC3** según la especificación de Kärkkäinen y Sanders.
2. Implementar **SAIS** según la estrategia de *induced sorting* de Nong et al.
3. Comprobar que ambos son correctos comparándolos con una implementación ingenua en tiempo cuadrático.
4. Medir el **tiempo de construcción** (*wall-clock*) en función de `n`.
5. Analizar el **uso de memoria RAM** y los **fallos de caché**.
6. Comparar ambos algoritmos sobre distintas distribuciones de entrada: texto aleatorio, código fuente, genomas y lenguaje natural.

## Estructura del repositorio

```
ProyectoEDA/
├── include/    # Cabeceras (interfaces de DC3, SAIS y utilidades)
├── src/        # Implementaciones: DC3, SAIS y versión ingenua de referencia
├── tests/      # Tests de correctitud (comparación con la versión ingenua)
├── bench/      # Programas de benchmark (tiempo, memoria, caché)
├── scripts/    # Scripts para descargar datasets, correr experimentos y graficar
├── data/       # Datasets de entrada (no se versionan; se descargan con scripts/)
├── results/    # Resultados de los experimentos (CSV, gráficas)
└── docs/       # Propuesta, informe final y notas
```

## Metodología experimental (planificada)

- **Entradas:** cadenas aleatorias con alfabetos de distinto tamaño, código fuente, secuencias de ADN y texto en lenguaje natural, con tamaños crecientes de `n`.
- **Métricas:**
  - tiempo de construcción;
  - pico de memoria (RSS);
  - fallos de caché, medidos con `perf stat` o `valgrind --tool=cachegrind`.

## Plan de trabajo

| Semana | Actividad |
|--------|-----------|
| 9 | Estudio de los papers, diseño y preparación del entorno (C++, compilación, *profiling*) |
| 10–11 | Implementación de DC3 (paso recursivo y fusión) + tests con cadenas pequeñas |
| 12–13 | Implementación de SAIS (tipos L/S, LMS, *induced sorting*) + validación contra la versión ingenua |
| 14 | Experimentos y benchmarks sobre los datasets |
| 15 | Análisis de resultados e informe final |

## Entregables

- Código fuente comentado de DC3 y SAIS.
- Scripts para reproducir los experimentos.
- Informe técnico con el análisis teórico, gráficas comparativas y conclusiones.

## Compilación y uso

_Pendiente: se documentará cuando exista el código._

## Referencias

1. Kärkkäinen, J., & Sanders, P. (2003). *Simple linear work suffix array construction.* ICALP 2003, LNCS 2719, pp. 943–955. Springer.
2. Nong, G., Zhang, S., & Chan, W. H. (2009). *Linear suffix array construction by almost pure induced-sorting.* Data Compression Conference (DCC 2009), pp. 193–202. IEEE.
