# Guía de Exposición y Defensa Oral: DC3 (Skew) vs. SA-IS

**Curso:** CS3014 — Estructuras de Datos Avanzadas (UTEC, 2026-2)  
**Tema:** Construcción de Suffix Arrays en Tiempo Lineal $\mathcal{O}(n)$: Análisis Teórico y Experimental de **DC3 (Kärkkäinen & Sanders, 2003)** y **SA-IS (Nong, Zhang & Chan, 2009)**  

---

## 1. Estructura Sugerida de Diapositivas (Guion Minuto a Minuto — 15 Minutos)

Esta estructura está diseñada para una exposición de **15 minutos** (más 5 minutos de preguntas del jurado), combinando rigor teórico, claridad sobre la adaptación en C++17 y resultados empíricos hasta $n = 2{,}000{,}000$.

| Diapositiva | Tiempo | Título de la Diapositiva | Qué decir exactamente (Discurso Clave) | Apoyo Visual Recomendado |
| :--- | :---: | :--- | :--- | :--- |
| **Slide 1** | `00:00 - 01:00` | **Título y Motivación: El Cuello de Botella de Indexación** | *"Los Suffix Arrays reemplazaron a los Suffix Trees en bioinformática y compresión porque reducen el espacio del índice final de $\sim 10\text{--}20\text{ B/char}$ a solo $4\text{ B/char}$. Sin embargo, ordenarlos ingenuamente toma $\mathcal{O}(n^2 \log n)$ porque comparar dos sufijos puede costar $\mathcal{O}(n)$ en cadenas con repeticiones largas. Hoy comparamos dos algoritmos que logran $\mathcal{O}(n)$: el paradigma algebraico **DC3 (2003)** y el paradigma estructural **SA-IS (2009)**."* | Tabla comparativa de complejidad y el contraejemplo periódico donde `Naive` tarda $183.66\text{ ms}$ en $n=10{,}000$ frente a $0.34\text{ ms}$ de SA-IS. |
| **Slide 2** | `01:00 - 03:30` | **Paradigma 1: DC3 / Skew (Kärkkäinen & Sanders, 2003)** | *"¿Por qué DC3 divide entre 3 y no entre 2? Si tomamos mitad y mitad, al fusionar un sufijo par con uno impar siempre caemos en otra paridad distinta sin resolver. En cambio, al muestrear las $\frac{2}{3}n$ posiciones con $i \bmod 3 \neq 0$, para cualquier par de sufijos $(i, j)$ existe un desplazamiento $k \in \{0, 1, 2\}$ donde **ambos** $(i+k)$ y $(j+k)$ pertenecen a la muestra $S_{12}$. Eso reduce cualquier comparación en la fusión a $\mathcal{O}(1)$ usando el arreglo inverso $\overline{SA}^{12}$."* | Diagrama de `s12` dividido en mitad izquierda ($i \bmod 3 = 1$) y mitad derecha ($i \bmod 3 = 2$), más las fórmulas de `leq` para pares y tripletas (`include/dc3.hpp`). |
| **Slide 3** | `03:30 - 06:30` | **Paradigma 2: SA-IS (Nong, Zhang & Chan, 2009)** | *"DC3 tiene una tasa de muestreo fija: su razón de reducción siempre es $\alpha = 2/3 \approx 0.667$, lo que da la recurrencia $T(n) = T(2n/3) + \mathcal{O}(n) = 3\mathcal{O}(n)$. SA-IS clasifica cada posición en tipo $S$ ($suf(i) < suf(i+1)$) o tipo $L$ ($suf(i) > suf(i+1)$) en una pasada de derecha a izquierda y solo muestrea las posiciones **LMS** (Leftmost S-type). Como nunca puede haber dos LMS adyacentes, $\alpha_{\text{SA-IS}} \le 0.5$ en el peor caso y $\sim 0.30\text{--}0.38$ en la práctica."* | Traza sobre `"mmiissiissiippii$"` mostrando vectores `t[]`, marcas `*` de posiciones LMS y las pasadas de cubetas (`induceSAl` de izq. a der. e `induceSAs` de der. a izq.). |
| **Slide 4** | `06:30 - 09:00` | **Adaptación en C++17: Seguridad Binaria y Memoria In-Place** | *"Adaptamos las implementaciones de referencia del Apéndice A de Kärkkäinen & Sanders (2003) y de Nong et al. (2009) a C++17 con contenedores RAII (`std::vector<int>`), soporte para todo el alfabeto binario `[0..255]` desplazando cada byte `+1` al rango `[1..256]` (`K = 256`, centinela `0`), y dos optimizaciones clave de los papers: (1) en DC3 reutilizamos `s12` para guardar el arreglo inverso $\overline{SA}^{12}$; y (2) en SA-IS empaquetamos la cadena reducida $S_1$ en la mitad derecha de `SA` usando `pos / 2`."* | Fragmentos cortos de `src/dc3.cpp` (líneas 97–108) y `src/sais.cpp` (líneas 75–94). |
| **Slide 5** | `09:00 - 11:30` | **Resultados Experimentales I: Escalamiento hasta $n = 2{,}000{,}000$** | *"Evaluamos 4 familias sintéticas hasta $n=2{,}000{,}000$ y 4 corpus de $1\text{ MB}$. En texto aleatorio uniforme (`random_ascii`), ambos algoritmos terminan en **profundidad 1** porque todas las tripletas y subcadenas LMS del nivel 1 son únicas; ahí el desempeño es prácticamente equivalente ($\text{Speedup} \approx 0.95\times\text{--}1.09\times$). En cambio, cuando hay estructura repetitiva (`dna_genome`, `source_code`, `periodic_str`), la recursión desciende de $3$ a $13$ niveles y la menor razón de reducción de SA-IS ($\sim 0.304\text{--}0.381$ vs. $0.667$) produce un speedup de **$1.32\times\text{ a }2.25\times$**."* | Figuras `results/fig1_time_scaling.png` y `results/fig2_speedup_by_family.png`. |
| **Slide 6** | `11:30 - 13:30` | **Resultados Experimentales II: Memoria Analítica, Recursión y Caché** | *"Medimos de forma analítica exacta la memoria de los vectores `int` de 32 bits reservados en el heap: DC3 requiere **$19.2\text{--}24.0\text{ B/char}$**, mientras que SA-IS requiere **$13.6\text{--}14.5\text{ B/char}$** (con una proyección teórica de **$5.2\text{--}6.1\text{ B/char}$** bajo el modelo compacto en bits del paper). Además, nuestro simulador de caché L1D (32 KB) / L2 (256 KB) muestra que DC3 realiza **$53\%\text{--}57\%$ de accesos indirectos** frente a **$36\%$** en SA-IS, generando casi el doble de fallos L1 por carácter ($24.23$ vs. $12.57$ en código fuente)."* | Figuras `results/fig3_memory_and_recursion.png` y `results/fig4_cache_and_corpus.png`. |
| **Slide 7** | `13:30 - 15:00` | **Conclusiones y Cuándo Elegir Cada Algoritmo** | *"En conclusión: **SA-IS** ofrece una ventaja importante para ejecución secuencial en CPU sobre genomas, código fuente y textos con repeticiones (hasta $2.25\times$ más rápido en nuestro corpus de código C++ de $1\text{ MB}$ y $\sim 40\%$ menos memoria en vectores de 32 bits). **DC3**, por su parte, es competitivo en entradas aleatorias de profundidad 1 y mantiene una ventaja arquitectónica cuando se requiere paralelismo masivo (GPU, MPI o memoria externa), donde la dependencia secuencial $SA[i] \to SA[i]-1$ de SA-IS limita la paralelización."* | Tabla resumen de criterios de elección algorítmica. |

---

## 2. Los 5 Puntos Técnicos Clave del Código que Demuestran Dominio

Si el jurado te pide abrir el código fuente durante la defensa, muestra cualquiera de estos **5 detalles de implementación**:

### Punto Clave 1: Desplazamiento `+1` (`[0..255] -> [1..256]`, `K = 256`) y Centinela `0`
- **Ubicación:** [`include/naive_sa.hpp`](../include/naive_sa.hpp#L75-L83) (`string_to_int_with_sentinel`) y funciones públicas [`src/dc3.cpp`](../src/dc3.cpp#L237-L244) (`build_dc3_sa`) y [`src/sais.cpp`](../src/sais.cpp#L207-L215) (`build_sais_sa`).
- **Por qué es vital:** En C++, `char` tiene signo por defecto en x86_64 (`-128` a `127`) y un `std::string` puede contener bytes arbitrarios en `[0..255]`, incluido el byte nulo `\0` (`0`). Si primero convertimos a `unsigned char` (`0..255`) pero no sumamos `+1`, un byte `\0` dentro de la cadena tendría valor `0` y colisionaría con el centinela `0`, rompiendo la invariante de unicidad del centinela en SA-IS y el orden de las tripletas de borde en DC3. Hacer `static_cast<int>(static_cast<unsigned char>(str[i])) + 1` desplaza todo el alfabeto binario a `[1, 256]` con `K = 256`, reservando el entero `0` exclusivamente para el centinela explícito `$` (`s[n] = 0` en SA-IS y el padding `s[n] = s[n+1] = s[n+2] = 0` en DC3).

### Punto Clave 2: El Sufijo "Fantasma" (`i = n`) y la Variable `t = n0 - n1` en DC3
- **Ubicación:** [`src/dc3.cpp`](../src/dc3.cpp#L46-L73) y [`src/dc3.cpp`](../src/dc3.cpp#L148-L155).
- **Por qué es vital:** Cuando $n \bmod 3 = 1$, hay un sufijo más en $i \bmod 3 = 0$ que en $i \bmod 3 = 1$ ($n_0 - n_1 = 1$). Para que los subarreglos de residuos `mod 1` y `mod 2` dentro de `s12` tengan exactamente la proporción requerida y para poder derivar `s0` directamente desde `SA12` sin casos borde, el Apéndice A de Kärkkäinen & Sanders agrega el índice ficticio `i = n` en el bucle `for (int i = 0, j = 0; i < n + (n0 - n1); ++i)` (cuya posición tiene residuo $n \bmod 3 = 1$ y cuya tripleta es `(0, 0, 0)`). Como `(0, 0, 0)` es lexicográficamente menor que cualquier sufijo real en `[1..256]`, este sufijo fantasma **siempre queda ordenado en `SA12[0]`**. Por eso el bucle de fusión en la línea 148 inicializa el cursor `t = n0 - n1` (es decir, `t = 1` cuando $n \bmod 3 = 1$), saltándose limpiamente `SA12[0]` para que `i = n` jamás aparezca en el `SA` final.

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
- **Por qué es vital:** Tras ordenar las `n1` subcadenas LMS en `SA[0 .. n1-1]`, necesitamos asignar a cada subcadena LMS su nombre lexicográfico `name` en el orden original en que aparecen en la cadena `s` para formar el problema reducido `s1`, sin reservar un nuevo arreglo en el heap. Como dos posiciones LMS nunca pueden ser adyacentes (si $i$ es LMS, $i-1$ es obligatoriamente tipo $L$), la división entera `pos / 2` mapea cualquier posición LMS $pos \in [1, n]$ a una celda única en el rango $[0, \lfloor n/2 \rfloor]$. Al escribir `SA[n1 + pos / 2] = name - 1`, almacenamos los nombres en la mitad derecha de `SA` sin colisiones y luego los compactamos hacia el final `SA[n + 1 - n1 .. n]`.

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

### Pregunta 5 (Sobre los Datos Experimentales): *"En sus resultados para `random_ascii` (incluso hasta $n=2{,}000{,}000$), tanto DC3 como SA-IS reportan `recursion_depth = 1` y un speedup cercano a $1.0\times$ ($0.95\times\text{--}1.09\times$), pero en `dna_genome` y `source_code` ambos reportan `recursion_depth = 4` y el speedup sube a $1.60\times\text{--}2.16\times$. ¿Por qué ocurre esto?"*
> **Respuesta:**  
> En `random_ascii`, el alfabeto tiene tamaño $|\Sigma| = 64$ caracteres uniformemente distribuidos.  
> - En **DC3**, el nivel 0 ordena tripletas de longitud 3 ($64^3 = 262{,}144$ combinaciones posibles) y entra al nivel recursivo 1, donde cada tripleta de nombres representa $9$ caracteres originales ($64^9 \approx 1.8 \times 10^{16}$ combinaciones). Incluso para $n = 2{,}000{,}000$, todas las tripletas del nivel 1 son únicas (`name == n02`), activando la parada temprana (`if (name < n02)` es falso en [`src/dc3.cpp`](../src/dc3.cpp#L97-L108)).  
> - En **SA-IS**, al nombrar las subcadenas LMS en texto aleatorio ($\alpha \approx 0.331$), en el nivel recursivo 1 todos los nombres también resultan únicos y termina de inmediato mediante bucket sort directo (`SA1[s1[i]] = i` en [`src/sais.cpp`](../src/sais.cpp#L99-L105)).  
> Como ambos algoritmos se detienen en **profundidad 1**, el ahorro en el tamaño de los niveles profundos de SA-IS no llega a activarse tanto, y el tiempo de ambos métodos resulta muy similar ($\text{Speedup} \approx 0.95\times\text{--}1.09\times$). En cambio, en **`dna_genome`** ($|\Sigma| = 4$, con solo $4^3 = 64$ tripletas posibles) y en **`source_code`** (con tokens e indentaciones repetidas), hay miles de empates en los primeros niveles que obligan a ambos algoritmos a recursar **4 a 5 niveles**, donde la razón de reducción $\alpha \approx 0.304\text{--}0.355$ de SA-IS frente al $0.667$ fijo de DC3 le da una ventaja de **$1.60\times\text{ a }2.25\times$**.

---

### Pregunta 6 (Sobre el Caso Periódico): *"En `periodic_str` ($n=2{,}000{,}000$), DC3 alcanza una profundidad de recursión de $13$ niveles mientras que SA-IS solo necesita $3$ niveles. ¿A qué se debe una diferencia tan marcada ($13$ vs. $3$)?"*
> **Respuesta:**  
> Nuestro generador `periodic_str` construye una cadena altamente periódica repitiendo un patrón fijo a lo largo de toda la entrada.  
> - **DC3** solo inspecciona ventanas fijas de longitud 3 en cada nivel de recursión y reduce el tamaño del problema a razón de $2/3$ por nivel. Para desambiguar sufijos periódicos idénticos hasta el final de la cadena, DC3 necesita seguir recursando hasta que $n \cdot (2/3)^d$ colapse cerca del final de la cadena, lo cual toma $d = 11$ niveles en $n = 500{,}000$, $d = 12$ en $n = 1{,}000{,}000$ y $d = 13$ en $n = 2{,}000{,}000$.  
> - **SA-IS**, por el contrario, particiona la cadena en subcadenas LMS de **longitud adaptativa** ($\alpha = 0.308$) y cada nivel recursivo concatena subcadenas LMS enteras, reduciendo exponencialmente más rápido el problema y resolviendo todas las repeticiones en **3 niveles**.

---

### Pregunta 7 (Sobre Medición de Memoria y Caché): *"¿Cómo midieron exactamente el consumo de memoria (`B/char`) y los fallos de caché L1/L2?"*
> **Respuesta:**  
> Separamos cuidadosamente tres métricas para evitar ambigüedades metodológicas:  
> 1. **Memoria Analítica de Vectores de 32 bits (`dc3_bytes_per_char` y `sais_bytes_per_char`):** Mediante los rastreadores `DC3Tracker` y `SAISTracker`, sumamos de forma analítica exacta los bytes de todos los `std::vector<int>` vivos simultáneamente en cada punto del árbol recursivo (`size() * sizeof(int)`), obteniendo $\approx 19.2\text{--}24.0\text{ B/char}$ en DC3 frente a $\approx 13.6\text{--}14.5\text{ B/char}$ en SA-IS.  
> 2. **Proyección Compacta (`sais_compact_bpc`):** Como nuestra implementación almacena el vector de tipos `t` como `std::vector<int>` por claridad pedagógica, calculamos adicionalmente la proyección teórica bajo el esquema compacto de Nong et al. (2009) (donde `s` de nivel 0 ocupa `1 B/char`, `SA` ocupa `4 B/char` y `t` se empaqueta a `1 bit/char`), obteniendo $\approx 5.18\text{--}6.30\text{ B/char}$. También consultamos `GetProcessMemoryInfo` (`PeakWorkingSetSize`) del sistema operativo para reportar el RSS global del proceso.  
> 3. **Simulador de Caché (`CacheHierarchySimulator`):** Instrumentamos una versión espejo de ambos algoritmos que registra cada lectura/escritura sobre un modelo LRU de **L1D (32 KB, 8 vías, línea de 64 B)** y **L2 (256 KB, 8 vías)**, demostrando que DC3 tiene un $53\%\text{--}57\%$ de accesos indirectos frente a $36\%$ en SA-IS, lo que duplica los fallos L1 por carácter en ADN y código fuente ($23.26$ vs. $11.85$ y $24.23$ vs. $12.57$).

---

### Pregunta 8 (Sobre Diseño de Sistemas): *"Si SA-IS supera a DC3 en memoria, fallos de caché y tiempo sobre textos estructurados, ¿en qué escenarios prácticos sigue prefiriéndose DC3?"*
> **Respuesta:**  
> En dos escenarios principales:  
> 1. **Entradas aleatorias de alta entropía sin repeticiones largas:** Como vimos en `random_ascii` ($n = 2{,}000{,}000$), cuando la recursión termina en profundidad 1, DC3 es igual de rápido (o ligeramente más rápido: $147.12\text{ ms}$ vs. $155.18\text{ ms}$) porque su fase de nivel 0 solo hace 4 pasadas simples de conteo y una fusión lineal.  
> 2. **Paralelismo masivo (GPU / MPI) y Memoria Externa (Disco):** SA-IS tiene una **dependencia de datos estrictamente secuencial** en sus fases de inducción (`induceSAl` e `induceSAs`): para inducir `j = SA[i] - 1` en la iteración $i$, es indispensable que una iteración anterior $i' < i$ haya escrito previamente `SA[i]`. DC3, en cambio, se compone exclusivamente de primitivas **Data-Parallel** (filtrado $\bmod 3 \neq 0$, Radix Sort paralelo, sumas prefijas y `merge` con comparaciones `leq` independientes en $\mathcal{O}(1)$), por lo que sigue siendo el algoritmo de referencia para GPUs, clústeres distribuidos y memoria externa.

---

## 4. Checklist de Comandos Rápidos para Demostración en Vivo

Si durante la exposición el jurado solicita ejecutar las pruebas o los benchmarks en vivo desde PowerShell, ten listos estos comandos en la raíz del proyecto (`c:\archivos de ema\2026-2\EDA\Proyecto_Final\ProyectoEDA`):

```powershell
# 1. Ejecutar la suite completa de pruebas unitarias (incluyendo bytes nulos '\0' y 1,000 cadenas binarias)
.\build\test_runner.exe

# 2. Ejecutar el benchmark sobre los 4 corpus de 1 MB en data/
.\build\benchmark.exe --only-files results/corpus_benchmark.csv data/corpus_random.txt data/corpus_dna.fasta data/corpus_source_code.txt data/corpus_periodic.txt

# 3. Ejecutar el simulador de caché L1/L2 en vivo
python scripts/profile_cache.py

# 4. Regenerar las 4 gráficas PNG en results/ desde los CSV
python scripts/plot_results.py
```
