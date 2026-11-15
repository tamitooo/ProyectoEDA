# Guía de Exposición y Defensa Oral: DC3 (Skew) vs. SA-IS

**Curso:** CS3014 — Estructuras de Datos Avanzadas (UTEC, 2026-2)  
**Tema:** Construcción de Suffix Arrays en Tiempo Lineal $\mathcal{O}(n)$: Análisis Teórico y Experimental de **DC3 (Kärkkäinen & Sanders, 2003)** y **SA-IS (Nong, Zhang & Chan, 2009)**  

---

## 1. Estructura Sugerida de Diapositivas (Guion Minuto a Minuto — 15 Minutos)

Esta estructura está diseñada para una exposición de **15 minutos** (más 5 minutos de preguntas del jurado), equilibrando rigor matemático, detalles de ingeniería en C++17 y resultados empíricos.

| Diapositiva | Tiempo | Título de la Diapositiva | Qué decir exactamente (Discurso Clave) | Apoyo Visual Recomendado |
| :--- | :---: | :--- | :--- | :--- |
| **Slide 1** | `00:00 - 01:00` | **Título y Motivación: El Cuello de Botella de Indexación** | *"Los Suffix Arrays reemplazaron a los Suffix Trees en bioinformática y compresión porque reducen el espacio de $\sim 20\text{--}40\text{ B/char}$ a solo $4\text{ B/char}$. Sin embargo, ordenarlos ingenuamente toma $\mathcal{O}(n^2 \log n)$ porque comparar dos sufijos puede costar $\mathcal{O}(n)$ en cadenas con repeticiones largas. Hoy comparamos dos hitos que logran $\mathcal{O}(n)$ puro: el paradigma algebraico **DC3 (2003)** y el paradigma estructural **SA-IS (2009)**."* | Tabla comparativa de complejidad y el contraejemplo periódico donde `Naive` tarda $153.91\text{ ms}$ en $n=10{,}000$ frente a $0.22\text{ ms}$ de SA-IS. |
| **Slide 2** | `01:00 - 03:30` | **Paradigma 1: DC3 / Skew (Kärkkäinen & Sanders, 2003)** | *"¿Por qué DC3 divide entre 3 y no entre 2? Si tomamos mitad y mitad, al fusionar un sufijo par con uno impar siempre caemos en otra paridad distinta sin resolver. En cambio, al muestrear las $\frac{2}{3}n$ posiciones con $i \bmod 3 \neq 0$, para cualquier par de sufijos $(i, j)$ existe un desplazamiento $k \in \{0, 1, 2\}$ donde **ambos** $(i+k)$ y $(j+k)$ pertenecen a la muestra $S_{12}$. Eso reduce cualquier comparación en la fusión a $\mathcal{O}(1)$ usando el arreglo inverso $\overline{SA}^{12}$."* | Diagrama de `s12` dividido en mitad izquierda ($i \bmod 3 = 1$) y mitad derecha ($i \bmod 3 = 2$), más las fórmulas de `leq` para pares y tripletas (`include/dc3.hpp`). |
| **Slide 3** | `03:30 - 06:30` | **Paradigma 2: SA-IS (Nong, Zhang & Chan, 2009)** | *"DC3 tiene un límite rígido: su razón de reducción siempre es $\alpha = 2/3 \approx 0.667$, lo que da la recurrencia $T(n) = T(2n/3) + \mathcal{O}(n) = 3\mathcal{O}(n)$. SA-IS rompe esa barrera clasificando cada posición en tipo $S$ ($suf(i) < suf(i+1)$) o tipo $L$ ($suf(i) > suf(i+1)$) en una sola pasada de derecha a izquierda. Solo muestrea los caracteres **LMS** (Leftmost S-type). Como nunca puede haber dos LMS adyacentes, $\alpha_{\text{SA-IS}} \le 0.5$ en el peor caso y $\sim 0.26\text{--}0.35$ en la práctica."* | Traza sobre `"mmiissiissiippii$"` mostrando vectores `t[]`, marcas `*` de posiciones LMS y las 3 pasadas de cubetas (`induceSAl` de izq. a der. e `induceSAs` de der. a izq.). |
| **Slide 4** | `06:30 - 09:00` | **Ingeniería en C++17: Trucos Algorítmicos y Memoria In-Place** | *"Implementamos ambos algoritmos desde cero en C++17 sin librerías externas. Destaco dos optimizaciones críticas de nuestra implementación: (1) en DC3 reutilizamos el mismo buffer `s12` para guardar el arreglo inverso $\overline{SA}^{12}$ sin alojar memoria extra; y (2) en SA-IS empaquetamos la cadena reducida $S_1$ en la mitad derecha de `SA` usando el desplazamiento entero `pos / 2`, porque la distancia mínima entre dos LMS es $2$. Esto evita reservar arreglos auxiliares en el heap."* | Fragmentos cortos de `src/dc3.cpp` (líneas 97–108) y `src/sais.cpp` (líneas 75–94). |
| **Slide 5** | `09:00 - 11:30` | **Resultados Experimentales I: Tiempo y Speedup por Familia** | *"Evaluamos 4 familias sintéticas hasta $n=500{,}000$ y 4 corpus reales. En texto aleatorio uniforme (`random_ascii`), ambos algoritmos terminan en **profundidad 1** porque casi todas las tripletas de DC3 y todas las subcadenas LMS de SA-IS se resuelven en el primer nivel; ahí SA-IS gana por $1.16\times$. Pero cuando hay estructura repetitiva (`dna_genome` y `source_code`), DC3 y SA-IS recursan hasta $4\text{--}10$ niveles: ahí el factor de reducción de SA-IS ($\sim 0.265\text{--}0.355$ vs. $0.667$ fijo de DC3) dispara el speedup a **$2.02\times\text{--}2.51\times$**."* | Figuras `results/fig1_time_scaling.png` y `results/fig2_speedup_by_family.png`. |
| **Slide 6** | `11:30 - 13:30` | **Resultados Experimentales II: Memoria RAM, Recursión y Caché L1/L2** | *"¿Por qué SA-IS es el doble de rápido si ambos son $\mathcal{O}(n)$? Primero, por memoria: DC3 consume **$20.2\text{--}23.9\text{ B/char}$**, mientras que SA-IS usa **$13.6\text{--}14.5\text{ B/char}$** (o **$5.2\text{--}6.3\text{ B/char}$** con `t[]` en bits). Segundo, instrumentamos un simulador de caché L1D (32 KB) / L2 (256 KB): DC3 realiza **$53\%\text{--}57\%$ de accesos indirectos** ($s[SA_{12}[i]]$) durante sus 4 `radixPass` y el `merge`, mientras que SA-IS solo hace **$36\%$** porque `induceSAl` e `induceSAs` barren `SA` secuencialmente. Eso reduce los fallos de caché L1 por carácter casi a la mitad ($12.57$ vs. $24.23$ en código fuente)."* | Figuras `results/fig3_memory_and_recursion.png` y `results/fig4_cache_and_corpus.png`. |
| **Slide 7** | `13:30 - 15:00` | **Conclusiones y Cuándo Elegir Cada Algoritmo** | *"En conclusión: **SA-IS** es el ganador indiscutible para ejecución secuencial en CPU sobre genomas, código fuente y texto natural (mitad de tiempo, mitad de fallos de caché L1 y casi un cuarto de la memoria en su variante compacta). **DC3**, sin embargo, mantiene una ventaja estructural clave cuando se requiere paralelismo masivo (GPU, MPI o memoria externa/disco), ya que sus `radixPass` y su `merge` no tienen la dependencia estrictamente secuencial $SA[i] \to SA[i]-1$ de la ordenación inducida."* | Resumen ejecutivo de ganadores por métrica. |

---

## 2. Los 5 Puntos Técnicos Clave del Código que Demuestran Dominio Total

Si el jurado te pide abrir el código fuente durante la defensa, muestra cualquiera de estos **5 detalles de implementación** para demostrar que no copiaste una caja negra, sino que comprendes cada invariante matemático:

### Punto Clave 1: Conversión `unsigned char` en `[1..255]` (`K = 255`) y Centinela `0`
- **Ubicación:** [`include/naive_sa.hpp`](../include/naive_sa.hpp#L75-L83) (`string_to_int_with_sentinel`) y funciones públicas [`src/dc3.cpp`](../src/dc3.cpp#L237-L244) (`build_dc3_sa`) y [`src/sais.cpp`](../src/sais.cpp#L207-L215) (`build_sais_sa`).
- **Por qué es vital:** En C++, el tipo `char` tiene signo por defecto en x86_64 (`-128` a `127`). Si un byte ASCII se convierte directamente a `int` y se usa como índice de cubeta en `c[key]` o `bkt[key]`, cualquier carácter con bit alto activado provocaría **lectura/escritura fuera de límites (Undefined Behavior)** por índice negativo. Hacer `static_cast<int>(static_cast<unsigned char>(str[i]))` garantiza que todos los caracteres no nulos de la cadena de entrada se mapeen limpiamente al rango `[1, 255]` con `K = 255`, reservando de forma exclusiva el entero `0` para el centinela explícito `$` (`s[n] = 0` en SA-IS y el padding `s[n] = s[n+1] = s[n+2] = 0` en DC3).

### Punto Clave 2: El Sufijo "Fantasma" (`i = n`) y la Variable `t = n0 - n1` en DC3
- **Ubicación:** [`src/dc3.cpp`](../src/dc3.cpp#L46-L73) y [`src/dc3.cpp`](../src/dc3.cpp#L148-L155).
- **Por qué es vital:** Cuando $n \bmod 3 = 1$, hay un sufijo más en $i \bmod 3 = 0$ que en $i \bmod 3 = 1$ ($n_0 - n_1 = 1$). Para que los subarreglos de residuos `mod 1` y `mod 2` dentro de `s12` tengan exactamente la proporción requerida y para poder derivar `s0` directamente desde `SA12` sin casos borde, el Apéndice A de Kärkkäinen & Sanders agrega el índice ficticio `i = n` en el bucle `for (int i = 0, j = 0; i < n + (n0 - n1); ++i)` (cuya posición tiene residuo $n \bmod 3 = 1$ y cuya tripleta es `(0, 0, 0)`). Como `(0, 0, 0)` es lexicográficamente menor que cualquier sufijo real, este sufijo fantasma **siempre queda ordenado en `SA12[0]`**. Por eso el bucle de fusión en la línea 148 inicializa el cursor `t = n0 - n1` (es decir, `t = 1` cuando $n \bmod 3 = 1$), saltándose limpiamente `SA12[0]` para que `i = n` jamás aparezca en el `SA` final.

### Punto Clave 3: La Lambda `GetI()` y el Desempate $\mathcal{O}(1)$ en el `merge` de DC3
- **Ubicación:** [`src/dc3.cpp`](../src/dc3.cpp#L148-L177) y [`include/dc3.hpp`](../include/dc3.hpp#L25-L34).
- **Por qué es vital:** Dentro de `SA12`, los elementos no son índices directos de la cadena original `s`, sino posiciones dentro del arreglo concatenado `s12` (donde la primera mitad $[0, n_0-1]$ corresponde a $i \bmod 3 = 1$ y la segunda mitad $[n_0, n_{02}-1]$ corresponde a $i \bmod 3 = 2$). La función lambda:
  ```cpp
  auto GetI = [&]() -> int {
      return (SA12[t] < n0) ? (SA12[t] * 3 + 1) : ((SA12[t] - n0) * 3 + 2);
  };
  ```
  reconstruye el índice real $i$ en $\mathcal{O}(1)$. Luego, al comparar $j \in S_0$ contra $i \in S_{12}$:
  - Si $i \bmod 3 = 1$ (`SA12[t] < n0`), basta comparar un **par** `(s[i], s12[SA12[t] + n0])` contra `(s[j], s12[j / 3])` porque $(i+1) \bmod 3 = 2 \in S_{12}$ y $(j+1) \bmod 3 = 1 \in S_{12}$.
  - Si $i \bmod 3 = 2$ (`SA12[t] >= n0`), `(i+1) mod 3 = 0` **no** está en $S_{12}$, por lo que necesitamos avanzar un carácter más y comparar una **tripleta** usando `leq` de 6 parámetros, donde `(i+2) mod 3 = 1` y `(j+2) mod 3 = 2` sí están ambos en $S_{12}$.

### Punto Clave 4: El Truco `pos / 2` para Empaquetar `s1` In-Place dentro de `SA` en SA-IS
- **Ubicación:** [`src/sais.cpp`](../src/sais.cpp#L75-L94).
- **Por qué es vital:** Tras ordenar las `n1` subcadenas LMS en `SA[0 .. n1-1]`, necesitamos asignar a cada subcadena LMS su nombre lexicográfico `name` en el orden original en que aparecen en la cadena `s` para formar el problema reducido `s1`, ¡pero sin hacer `new int[n]`! Como dos posiciones LMS nunca pueden ser adyacentes (si $i$ es LMS, $i-1$ es obligatoriamente tipo $L$), la división entera `pos / 2` mapea cualquier posición LMS $pos \in [1, n]$ a una celda única en el rango $[0, \lfloor n/2 \rfloor]$. Al escribir `SA[n1 + pos / 2] = name - 1`, almacenamos los nombres en la mitad derecha de `SA` sin colisiones y luego los compactamos hacia el final `SA[n + 1 - n1 .. n]`.

### Punto Clave 5: Por Qué la Comparación de Subcadenas LMS (`lmsSubstringsAreEqual`) Verifica Caracteres Y Tipos L/S
- **Ubicación:** [`include/sais.hpp`](../include/sais.hpp#L39-L52) y [`src/sais.cpp`](../src/sais.cpp#L79-L88).
- **Por qué es vital:** En la Definición 2.3 / Lema 2.3 de Nong, Zhang & Chan (2009), dos subcadenas LMS $s[i..i']$ y $s[j..j']$ solo son equivalentes si coinciden tanto en **caracteres** (`s[pos1 + d] == s[pos2 + d]`) como en **tipos** (`t[pos1 + d] == t[pos2 + d]`) en cada desplazamiento $d$, y ambas terminan en un carácter LMS al mismo tiempo (`d > 0 && (isLMS(pos1 + d, t) || isLMS(pos2 + d, t))`). Además, aunque parezca un bucle anidado, el costo total de todas las llamadas a `lmsSubstringsAreEqual` en un nivel de recursión está acotado por $\mathcal{O}(n)$ porque solo comparamos subcadenas LMS adyacentes en `SA[0 .. n1-1]` y las subcadenas LMS son disjuntas salvo en sus extremos.

---

## 3. Banco de 8 Preguntas Difíciles de Jurado y Respuestas Exactas

### Pregunta 1 (Sobre DC3): *"¿Por qué DC3 divide las posiciones módulo 3 y toma $\frac{2}{3}n$ elementos en lugar de dividir módulo 2 (pares e impares), lo cual daría una recurrencia más pequeña $T(n/2)$?"*
> **Respuesta:**  
> Si dividiéramos módulo 2, tendríamos los sufijos pares $S_{\text{par}}$ y los impares $S_{\text{impar}}$. Supongamos que ordenamos recursivamente $S_{\text{impar}}$ en $T(n/2)$. Luego podríamos ordenar $S_{\text{par}}$ en $\mathcal{O}(n)$ haciendo Radix Sort de `(s[2k], rank(2k+1))`. El problema insalvable aparece en la **fase de fusión (`merge`)**: cuando queremos comparar un sufijo par $2k \in S_{\text{par}}$ contra un sufijo impar $2m+1 \in S_{\text{impar}}$, comparamos sus primeros caracteres `s[2k]` vs `s[2m+1]`. Si empatan, pasamos a comparar los sufijos desplazados $2k+1$ (impar) contra $2m+2$ (par): ¡uno sigue siendo par y el otro sigue siendo impar! Nunca caemos en dos sufijos que pertenezcan ambos al conjunto ya ordenado $S_{\text{impar}}$.  
> En cambio, al tomar módulo 3 y muestrear **dos** clases de residuo ($S_{12} = \{i : i \bmod 3 \in \{1, 2\}\}$), por principio del palomar, entre tres desplazamientos consecutivos $(i+k, j+k)$ para $k \in \{0, 1, 2\}$, a lo sumo uno de los dos índices puede caer en $\bmod 3 = 0$ en cada paso; por ende, en a lo sumo 2 pasos ($k=1$ o $k=2$) **ambos** $(i+k)$ y $(j+k)$ caen garantizadamente dentro de $S_{12}$, permitiendo desempatar en $\mathcal{O}(1)$.

---

### Pregunta 2 (Sobre SA-IS): *"En `induceSAl`, ustedes recorren `SA` de izquierda a derecha leyendo `j = SA[i] - 1` y escribiendo `j` más adelante en el mismo arreglo `SA` que están recorriendo. ¿Por qué esto es correcto y cómo saben que `j` siempre se escribe a la derecha de `i` y nunca a su izquierda?"*
> **Respuesta:**  
> Esa es la propiedad fundamental del Lema 2.1 de Nong et al. (2009). En `induceSAl`, solo inducimos la posición $j = SA[i] - 1$ cuando $j$ es de **tipo $L$** (`t[j] == L_TYPE`). Por definición de tipo $L$:
> $$\text{suf}(j) > \text{suf}(j + 1) = \text{suf}(SA[i])$$
> Como el sufijo $j$ es **estrictamente mayor** lexicográficamente que el sufijo $j+1 = SA[i]$, su posición correcta en el Suffix Array ordenado debe estar obligatoriamente en un índice $k > i$ (es decir, estrictamente a la derecha del cursor actual $i$). Por lo tanto, al recorrer `SA` de izquierda a derecha ($i = 0 \to n$), cada nuevo sufijo tipo $L$ inducido se deposita más adelante y será leído oportunamente cuando el cursor $i$ llegue a esa cubeta, propagando cadenas completas de sufijos tipo $L$ ($L \to L \to L$) en una sola pasada lineal. Análogamente, en `induceSAs`, si $j = SA[i]-1$ es de tipo $S$, entonces $\text{suf}(j) < \text{suf}(j+1)$, por lo que $j$ siempre se escribe a la izquierda ($k < i$) y por eso debemos recorrer `SA` de derecha a izquierda ($i = n \to 0$).

---

### Pregunta 3 (Sobre SA-IS): *"¿Cómo es posible que el Paso 1 de SA-IS ordene correctamente todas las subcadenas LMS si al principio ustedes colocan los índices LMS en las colas de sus cubetas S en un orden completamente arbitrario?"*
> **Respuesta:**  
> Porque las tres pasadas de inducción actúan como un **Radix Sort implícito de longitud variable** sobre las subcadenas LMS. Recordemos que toda subcadena LMS $s[i..i']$ tiene la estructura $S^+ L^+ S$ (comienza en un carácter LMS de tipo $S$, puede tener varios caracteres $S$, luego un bloque de caracteres $L$, y termina en el siguiente carácter LMS $i'$).  
> 1. Al insertar los LMS al final de sus cubetas, todos los caracteres finales $i'$ quedan agrupados por su primer carácter $s[i']$ (el carácter menos significativo / más a la derecha de la subcadena LMS anterior).  
> 2. Cuando `induceSAl` barre de izquierda a derecha, toma esos finales LMS y propaga hacia atrás todo el bloque de prefijos de **tipo $L$** ($L^+$) en orden lexicográfico exacto.  
> 3. Finalmente, cuando `induceSAs` barre de derecha a izquierda desde las posiciones tipo $L$ recién ordenadas, propaga hacia atrás el bloque inicial de **tipo $S$** ($S^+$) hasta llegar al inicio $i$ de cada subcadena LMS.  
> Así, sin comparar caracteres directamente, las tres pasadas de cubetas dejan todas las subcadenas LMS ordenadas por su contenido completo en tiempo $\mathcal{O}(n)$.

---

### Pregunta 4 (Sobre Complejidad en SA-IS): *"La función `lmsSubstringsAreEqual` tiene un bucle que compara carácter por carácter dos subcadenas LMS. Si hay $\mathcal{O}(n)$ subcadenas LMS y cada una puede medir $\mathcal{O}(n)$, ¿por qué ese paso no degrada a $\mathcal{O}(n^2)$?"*
> **Respuesta:**  
> Porque las subcadenas LMS **no se traslapan** en su interior: la cadena $s[0..n]$ queda particionada en subcadenas LMS consecutivas $s[i_1..i_2], s[i_2..i_3], \dots, s[i_{k-1}..i_k]$ que solo comparten sus puntos extremos LMS ($i_2, i_3, \dots$). Por lo tanto, la suma de las longitudes de **todas** las subcadenas LMS de la cadena es menor o igual a $2n$:
> $$\sum_{r=1}^{n_1} |LMS_r| \le 2n$$
> En el bucle de nombramiento ([`src/sais.cpp`](../src/sais.cpp#L79-L88)), solo comparamos cada subcadena LMS en `SA[i]` contra su **vecina inmediata anterior** `prev`. Cada subcadena LMS participa en a lo sumo dos comparaciones (una como `pos` y otra como `prev`), y cada llamada a `lmsSubstringsAreEqual(s, t, n, prev, pos)` ([`include/sais.hpp`](../include/sais.hpp#L39-L52)) avanza a lo sumo $\min(|LMS_{\text{prev}}|, |LMS_{\text{pos}}|)$ pasos antes de encontrar una diferencia o el carácter LMS final. Sumando sobre todos los $i \in [0, n_1-1]$, el número total de iteraciones está acotado superiormente por $2 \sum |LMS_r| \le 4n = \mathcal{O}(n)$.

---

### Pregunta 5 (Sobre los Datos Experimentales): *"En sus resultados para `random_ascii` ($n=500{,}000$), tanto DC3 como SA-IS reportan `recursion_depth = 1`, pero en `dna_genome` ambos reportan `recursion_depth = 4`. ¿Por qué en texto aleatorio la recursión se detiene en el nivel 1?"*
> **Respuesta:**  
> En `random_ascii`, el alfabeto tiene tamaño $|\Sigma| = 64$ caracteres uniformemente distribuidos.  
> - En **DC3**, el nivel 0 ordena tripletas de longitud 3 ($64^3 = 262{,}144$ combinaciones posibles) y entra al nivel recursivo 1, donde cada par o tripleta de nombres representa $6$ o $9$ caracteres originales ($64^6 \approx 6.87 \times 10^{10}$ combinaciones). Para $n = 500{,}000$ ($n_{02} = 333{,}334$), todas las tripletas del nivel 1 son únicas (`name == n02`), activando la parada temprana (`if (name < n02)` es falso en [`src/dc3.cpp`](../src/dc3.cpp#L97-L108) y no necesita profundizar más).  
> - En **SA-IS**, en texto aleatorio uniforme la probabilidad de que un carácter sea tipo $S$ es $\approx 1/2$, y las subcadenas LMS tienen longitudes variadas con $64^k$ combinaciones posibles. Al nombrar las $n_1 \approx 165{,}602$ subcadenas LMS ($\alpha = 0.331$), en el nivel recursivo 1 todos los nombres resultan únicos, por lo que termina de inmediato mediante bucket sort directo (`SA1[s1[i]] = i` en [`src/sais.cpp`](../src/sais.cpp#L99-L105)).  
> En cambio, en **`dna_genome`** el alfabeto tiene solo $|\Sigma| = 4$ bases (`A, C, G, T`). Con solo $4^3 = 64$ tripletas posibles en DC3 y muy pocas subcadenas LMS cortas posibles en SA-IS, para $n=500{,}000$ hay miles de colisiones de nombres en los primeros niveles, obligando a ambos algoritmos a recursar **4 niveles** hasta desambiguar todas las repeticiones.

---

### Pregunta 6 (Sobre el Caso Periódico): *"En `periodic_str` ($n=500{,}000$), DC3 alcanza una profundidad de recursión de $11$ niveles mientras que SA-IS solo necesita $3$ niveles. ¿A qué se debe una diferencia tan extrema ($11$ vs. $3$)?"*
> **Respuesta:**  
> Nuestro generador `periodic_str` construye una cadena altamente periódica repitiendo un bloque base con perturbaciones espaciadas.  
> - **DC3** solo "ve" ventanas fijas de longitud 3 en cada nivel de recursión (en el nivel $d$, cada símbolo representa una subcadena de longitud $3^d$). Para distinguir repeticiones periódicas largas cuyas diferencias están separadas por cientos o miles de caracteres, DC3 necesita que $3^d$ supere el periodo de repetición y que $n \cdot (2/3)^d$ colapse, lo cual requiere $d = 11$ niveles ($3^{11} = 177{,}147$).  
> - **SA-IS**, por el contrario, particiona la cadena en subcadenas LMS de **longitud adaptativa**. En una cadena periódica o con corridas largas de caracteres iguales, los bloques monótonos $L^+$ y $S^+$ se agrupan en subcadenas LMS más largas ya desde el nivel 0 (por eso su razón de reducción baja a $\alpha = 0.308$). Además, cada nivel recursivo de SA-IS concatena múltiples subcadenas LMS anteriores en super-bloques LMS cuyo alcance efectivo crece mucho más rápido que $3^d$, resolviendo todas las repeticiones en tan solo **3 niveles**.

---

### Pregunta 7 (Sobre Jerarquía de Caché): *"Ambos algoritmos usan Bucket Sort / Radix Sort sobre arreglos de enteros. ¿Por qué su simulador de caché muestra que DC3 tiene casi el doble de fallos de caché L1 por carácter que SA-IS ($24.23$ vs. $12.57$ en `source_code`)?"*
> **Respuesta:**  
> La diferencia radica en la **indirección de memoria**.  
> - En `radixPass(a, b, r, n, K)` de **DC3**, para contar frecuencias y distribuir elementos leemos `r[a[i]]`. Aquí `a[i]` contiene índices permutados de la cadena que saltan por todo el arreglo `r` (que para $n=200{,}000\text{--}500{,}000$ ocupa de $800\text{ KB}$ a $2\text{ MB}$, superando ampliamente la caché L1D de $32\text{ KB}$ y la L2 de $256\text{ KB}$). Cada lectura `r[a[i]]` salta a una línea de caché de 64 bytes distinta. DC3 ejecuta **4 llamadas a `radixPass`** por nivel más el `merge` que consulta `s12[SA12[t] + n0]` y `s12[j / 3]`, acumulando un **$55.47\%$ de accesos indirectos**.  
> - En **SA-IS**, el barrido sobre `SA[i]` en `induceSAl` e `induceSAs` es $100\%$ secuencial (con prefetcher de hardware ideal), y la mayoría de iteraciones ni siquiera tocan `s[]` si `SA[i] <= 0` o si el tipo `t[SA[i]-1]` no corresponde a la pasada actual (por ejemplo, en `induceSAl` si `t[j] == S_TYPE` se ignora sin leer `s[j]`). Como resultado, solo el **$36.22\%$** de los accesos de SA-IS son indirectos, reduciendo los fallos L1/char de $24.23$ a $12.57$ ($1.93\times$ menos).

---

### Pregunta 8 (Sobre Diseño de Sistemas): *"Si SA-IS le gana a DC3 en tiempo, memoria RAM, profundidad de recursión y fallos de caché en todas las pruebas, ¿por qué sigue estudiándose y usándose DC3 en la industria?"*
> **Respuesta:**  
> Porque SA-IS tiene una **dependencia de datos estrictamente secuencial** en sus fases de inducción (`induceSAl` e `induceSAs`): para inducir la posición `j = SA[i] - 1` en la iteración $i$, es indispensable que una iteración anterior $i' < i$ haya escrito previamente el valor de `SA[i]` en esa celda. No puedes dividir el bucle `for (int i = 0; i <= n; ++i)` de `induceSAl` entre 10,000 hilos de una GPU ni entre 100 nodos de un clúster MPI.  
> **DC3**, en cambio, se compone exclusivamente de operaciones **Data-Parallel** (paralelizables de forma masiva y en memoria externa / disco):  
> 1. Filtrar $i \bmod 3 \neq 0$.  
> 2. Ordenar tripletas con Radix Sort paralelo.  
> 3. Calcular sumas prefijas (`prefix sums`) para asignar nombres lexicográficos `name`.  
> 4. Fusionar dos arreglos ordenados (`SA0` y `SA12`) donde cada comparación `leq` es independiente y cuesta $\mathcal{O}(1)$.  
> Por eso, en un solo hilo de CPU **SA-IS es el rey**, pero en GPUs (CUDA), supercomputadoras distribuidas (MPI) o construcción *out-of-core* en disco duro para terabytes de datos, **DC3 (Skew)** sigue siendo la arquitectura algorítmica preferida.

---

## 4. Checklist de Comandos Rápidos para Demostración en Vivo

Si durante la exposición el jurado solicita ejecutar las pruebas o los benchmarks en vivo desde PowerShell, ten listos estos comandos en la raíz del proyecto (`c:\archivos de ema\2026-2\EDA\Proyecto_Final\ProyectoEDA`):

```powershell
# 1. Ejecutar la suite completa de pruebas unitarias (Etapas 1, 2 y 3)
.\build\test_runner.exe

# 2. Ejecutar el benchmark sobre los 4 corpus reales en data/ (~1 segundo)
.\build\benchmark.exe --only-files results/corpus_benchmark.csv data/corpus_random.txt data/corpus_dna.fasta data/corpus_source_code.txt data/corpus_periodic.txt

# 3. Ejecutar el simulador de caché L1/L2 en vivo (~0.5 segundos)
python scripts/profile_cache.py

# 4. Regenerar las 4 gráficas PNG en results/ desde los CSV (~1 segundo)
python scripts/plot_results.py
```
