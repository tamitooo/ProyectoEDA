# Base Teórica Completa de las Fuentes Principales (DC3 y SA-IS)

---

# FUENTE 1: Algoritmo Skew / DC3 (Kärkkäinen y Sanders, 2003)
**Título original:** *Simple Linear Work Suffix Array Construction* (ICALP 2003).

## 1. Contexto e Idea Central (Sección 1)
* **Limitación previa:** Los árboles de sufijos podían construirse en tiempo lineal $\mathcal{O}(n)$ (por ejemplo, con el algoritmo de Farach para alfabetos enteros), pero sufren de un alto consumo de memoria. Por otro lado, los algoritmos previos que construían directamente el Arreglo de Sufijos sin usar árboles tardaban $\mathcal{O}(n \log n)$.
* **Por qué dividir en tercios ($\bmod 3$) en vez de mitades ($\bmod 2$):** El algoritmo de Farach para árboles de sufijos divide las posiciones en impares (la mitad de la cadena) y pares (la otra mitad), resolviendo recursivamente y fusionando ambos árboles. Sin embargo, fusionar dos mitades sin las propiedades estructurales de un árbol de sufijos es sumamente complejo (como ocurre en el algoritmo contemporáneo de Kim et al.).
* **El hallazgo del algoritmo Skew (DC3):** Si en el primer paso se toman **dos tercios** de los sufijos (aquellos que inician en posiciones $i \bmod 3 \neq 0$) en lugar de la mitad, la fase final de fusión (*merge*) con el tercio restante ($i \bmod 3 = 0$) se vuelve casi trivial y se resuelve mediante comparaciones directas en tiempo constante $\mathcal{O}(1)$.

## 2. Modelos de Computación y Complejidades (Sección 1 y Sección 3)
El documento define tres tipos de alfabeto $\Sigma$ para una cadena de longitud $n$:
1. **Alfabeto constante:** $|\Sigma| = \mathcal{O}(1)$.
2. **Alfabeto entero:** Los caracteres son enteros dentro de un rango polinomial $n^{\mathcal{O}(1)}$ (permite usar *Radix Sort* en tiempo lineal).
3. **Alfabeto general:** Solo se asume que dos caracteres pueden compararse en tiempo constante $\mathcal{O}(1)$.

### Garantías de Rendimiento del Algoritmo Skew (Tabla 1 y Teorema 2)
| Modelo de Computación | Complejidad del Algoritmo Skew | Tipo de Alfabeto |
| :--- | :--- | :--- |
| **RAM Estándar** | Tiempo $\mathcal{O}(n)$ | Entero |
| **Memoria Externa** ($D$ discos, bloque $B$, memoria rápida $M$) | $\mathcal{O}\left(\frac{n}{DB} \log_{M/B} \frac{n}{B}\right)$ I/Os y $\mathcal{O}\left(n \log_{M/B} \frac{n}{B}\right)$ trabajo interno | Entero |
| **Cache-Oblivious** ($M/B$ bloques de caché de tamaño $B$) | $\mathcal{O}\left(\frac{n}{B} \log_{M/B} \frac{n}{B}\right)$ fallos de caché (*cache faults*) | General |
| **BSP** ($P$ procesadores, $h$-relación en tiempo $L+gh$) | Tiempo $\mathcal{O}\left(\frac{n \log n}{P} + L \log^2 P + \frac{gn \log n}{P \log(n/P)}\right)$ | General |
| **BSP** (con $P = \mathcal{O}(n^{1-\epsilon})$ procesadores) | Tiempo $\mathcal{O}(n/P + L \log^2 P + gn/P)$ (trabajo lineal) | Entero |
| **EREW-PRAM** | Tiempo $\mathcal{O}(\log^2 n)$ y trabajo $\mathcal{O}(n \log n)$ | General |
| **priority-CRCW-PRAM** | Tiempo $\mathcal{O}(\log^2 n)$ y trabajo $\mathcal{O}(n)$ (aleatorizado) | Constante |

## 3. Funcionamiento Matemático Detallado del Algoritmo Skew (Sección 2)

### Convenciones y Precondiciones
* La cadena de entrada $s[0, n)$ se indexa desde $0$ sobre el alfabeto entero $\Sigma = [1, n]$.
* El sufijo que comienza en la posición $i$ se denota como $S_i = s[i, n)$.
* Se adopta la convención de agregar caracteres cero al final de cualquier cadena considerada ($\alpha[|\alpha|] = \alpha[|\alpha|+1] = 0$, y tres ceros `s[n] = s[n+1] = s[n+2] = 0` en la implementación práctica) para evitar desbordamientos al formar tripletas.

### Paso 1: Ordenar los sufijos con posición $i \bmod 3 \neq 0$
1. **Agrupación en tripletas:** Se seleccionan todas las posiciones $i$ tales que $i \bmod 3 = 1$ o $i \bmod 3 = 2$ (que representan $\frac{2}{3}n$ elementos) y se observa la tripleta de caracteres $s[i, i+2] = (s[i], s[i+1], s[i+2])$.
2. **Ordenamiento lineal con Radix Sort:** Se ordenan las tripletas en tiempo $\mathcal{O}(n)$ realizando tres pasadas estables de ordenamiento por conteo (*radix sort*): primero por la posición $i+2$, luego por $i+1$ y finalmente por $i$.
3. **Nombramiento lexicográfico:** Se recorre la lista de tripletas ya ordenadas y se asigna a cada una un número entero (su "nombre lexicográfico") $s'_i \in [1, 2n/3]$. Si una tripleta es idéntica a la anterior en el orden, recibe el mismo número; si es diferente, el contador aumenta en $1$ (de modo que la $k$-ésima tripleta distinta recibe el nombre $k$).
4. **Construcción de la cadena reducida $s^{12}$:** Se concatenan todos los nombres $s'_i$ de las posiciones $i \bmod 3 = 1$ seguidos de todos los nombres $s'_i$ de las posiciones $i \bmod 3 = 2$:
   $$s^{12} = [s'_i : i \bmod 3 = 1] \circ [s'_i : i \bmod 3 = 2]$$
   Los ceros de relleno al final de $s$ aseguran que el último elemento de la primera mitad sea único en $s^{12}$, impidiendo que un sufijo de la primera mitad se confunda al cruzar hacia la segunda mitad.
5. **Caso Base vs. Llamada Recursiva:**
   * Si todas las tripletas obtuvieron nombres distintos, no hay empates y el orden de los sufijos $i \bmod 3 \neq 0$ ya es definitivo (se construye $SA^{12}$ directamente).
   * Si al menos dos tripletas empataron (hay menos nombres únicos que elementos en $s^{12}$), se invoca recursivamente al algoritmo Skew sobre la cadena $s^{12}$ (cuyo tamaño es $\frac{2}{3}n$) para obtener su arreglo de sufijos $SA^{12}$.
6. **Cálculo del arreglo inverso ($\overline{SA}^{12}$):** A partir de $SA^{12}$, se precalcula un arreglo inverso $\overline{SA}^{12}$ definido como $\overline{SA}^{12}[i] = j + 1$ si $SA^{12}[j] = i$. Este arreglo permite saber en tiempo constante $\mathcal{O}(1)$ qué sufijo es mayor entre cualesquiera dos posiciones que no sean múltiplos de 3.

### Paso 2: Ordenar los sufijos con posición $i \bmod 3 = 0$
* Cualquier sufijo $S_i$ con $i \bmod 3 = 0$ puede verse como el par formado por su primer carácter y el sufijo siguiente: $(s[i], S_{i+1})$.
* Como $i \bmod 3 = 0$, entonces $(i+1) \bmod 3 = 1$. Esto significa que el orden relativo de todos los sufijos $S_{i+1}$ **ya fue calculado en el Paso 1** y está implícito en $SA^{12}$.
* Por lo tanto, basta con extraer de $SA^{12}$ aquellos índices que corresponden a posiciones $i+1$ (con $i \bmod 3 = 0$) conservando su orden, y aplicarles **una única pasada estable de Radix Sort** usando como clave el carácter $s[i]$.

### Paso 3: Fusión (*Merge*) de los dos grupos en tiempo $\mathcal{O}(n)$
Para fusionar el arreglo de los múltiplos de 3 ($SA^0$) con el arreglo de los no múltiplos de 3 ($SA^{12}$), necesitamos comparar cualquier sufijo $S_j$ (donde $j \bmod 3 = 0$) contra cualquier sufijo $S_i$ (donde $i \bmod 3 \neq 0$) en tiempo $\mathcal{O}(1)$:
* **Caso 1 ($i \bmod 3 = 1$):** Expresamos ambos sufijos como pares: $S_i = (s[i], S_{i+1})$ y $S_j = (s[j], S_{j+1})$.
  * Primero comparamos los caracteres iniciales $s[i]$ y $s[j]$.
  * Si son iguales, necesitamos comparar $S_{i+1}$ contra $S_{j+1}$. Como $(i+1) \bmod 3 = 2$ y $(j+1) \bmod 3 = 1$, **ninguno de los dos es múltiplo de 3**, por lo que ambos están en $\overline{SA}^{12}$ y su orden relativo se consulta en $\mathcal{O}(1)$.
* **Caso 2 ($i \bmod 3 = 2$):** Si sólo avanzáramos un carácter, $(i+1) \bmod 3$ sería $0$ (no está en $\overline{SA}^{12}$). Por eso expresamos ambos sufijos como tripletas: $S_i = (s[i], s[i+1], S_{i+2})$ y $S_j = (s[j], s[j+1], S_{j+2})$.
  * Comparamos primero $s[i]$ con $s[j]$, y si empatan, $s[i+1]$ con $s[j+1]$.
  * Si vuelven a empatar, comparamos $S_{i+2}$ contra $S_{j+2}$. Como $(i+2) \bmod 3 = 1$ y $(j+2) \bmod 3 = 2$, nuevamente **ambos pertenecen al grupo $\bmod 3 \neq 0$**, resolviendo la comparación en $\mathcal{O}(1)$ mediante $\overline{SA}^{12}$.

### Traza del Ejemplo del Paper (Figura 1): `s = mississippi`
1. **Entrada ($n=11$):** Posiciones `0:m, 1:i, 2:s, 3:s, 4:i, 5:s, 6:s, 7:i, 8:p, 9:p, 10:i`.
2. **Tripletas $\bmod 3 = 1$ (posiciones 1, 4, 7, 10):** `iss`, `iss`, `ipp`, `i00` $\rightarrow$ reciben los nombres lexicográficos `3, 3, 2, 1`.
3. **Tripletas $\bmod 3 = 2$ (posiciones 2, 5, 8):** `ssi`, `ssi`, `ppi` $\rightarrow$ reciben los nombres lexicográficos `5, 5, 4`.
4. **Cadena concatenada $s^{12}$:** `[3, 3, 2, 1, 5, 5, 4]` (como el `3` y el `5` se repiten, se hace llamada recursiva).
5. **Resultado recursivo $SA^{12}$:** `[3, 2, 1, 0, 6, 5, 4]`, que corresponde a los sufijos originales en las posiciones `[10, 7, 4, 1, 8, 5, 2]`.
6. **Arreglo inverso $\overline{SA}^{12}$:** `[4, 3, 2, 1, 7, 6, 5]`.
7. **Sufijos $\bmod 3 = 0$ ordenados:** Posiciones `[0, 9, 6, 3]`.
8. **Resultado tras el Merge ($SA$):** `[10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2]`.

### Demostración de Complejidad, LCP y Cubiertas de Diferencias (Secciones 2, 4 y 5)
* **Teorema 1 (Tiempo Lineal):** Todos los pasos fuera de la recursión toman $\mathcal{O}(n)$ y el llamado recursivo se ejecuta sobre un tamaño de $\lceil 2n/3 \rceil$. La recurrencia $T(n) = T(\lceil 2n/3 \rceil) + \mathcal{O}(n)$ con $T(n) = \mathcal{O}(1)$ para $n < 3$ tiene como solución $T(n) = \mathcal{O}(n)$ (por serie geométrica $1 + 2/3 + 4/9 + \dots = 3$).
* **Extensión al arreglo LCP (Sección 4, Teoremas 3 y 4, Lema 1):** El algoritmo puede calcular simultáneamente el arreglo *Longest Common Prefix* ($LCP[i] = \text{lcp}(SA[i], SA[i+1])$) en tiempo $\mathcal{O}(n)$ a partir de $LCP^{12}$ preprocesando bloques de tamaño $\log n$ para responder consultas de mínimo en rango (RMQ) en tiempo $\mathcal{O}(1)$, apoyado en que ningún sufijo participa en más de dos consultas LCP en el nivel superior.
* **Generalización con Cubiertas de Diferencias (Sección 5):** El conjunto $\{1, 2\}$ funciona porque es una **cubierta de diferencias módulo 3** (*difference cover modulo $v$*): un subconjunto $D \subseteq [0, v)$ tal que cualquier residuo $i \in [0, v)$ puede obtenerse como $i \equiv k - j \pmod v$ con $j, k \in D$. Por ejemplo, $\{1\}$ no sirve para módulo 2, pero $\{3, 5, 6\}$ es una cubierta de diferencias módulo 7 que permitiría reducir el problema a $\frac{3}{7}n$.
* **Detalles de implementación en C++ (Apéndice A):** Cuando la longitud $n$ cumple $n \bmod 3 = 1$, se cumple que $n_0 = \lfloor(n+2)/3\rfloor$ supera en $1$ a $n_1 = \lfloor(n+1)/3\rfloor$. Para simplificar los índices entre las dos mitades de `s12`, se incluye un sufijo ficticio (*dummy*) en la posición $i = n$ durante el Paso 1, y luego se salta ese elemento inicial al comenzar el bucle del `merge` inicializando el cursor $t = n_0 - n_1$.

---

# FUENTE 2: Algoritmo SA-IS (Nong, Zhang y Chan, 2009)
**Título original:** *Linear Suffix Array Construction by Almost Pure Induced-Sorting* (IEEE DCC 2009).

## 1. Motivación y Mejoras frente al Algoritmo de Ko y Aluru (Sección 1)
* **El antecedente (Algoritmo KA, 2003):** Ko y Aluru propusieron reducir la cadena tomando las subcadenas de tipo S (*S-substrings*), resolver recursivamente y usar *ordenamiento inducido* en el paso final. Sin embargo, ordenar las subcadenas de tipo S en el Paso 1 era su cuello de botella: requería mantener "listas de distancias S" (*S-distance lists*) con alto costo espacial y más de 1000 líneas de código en C.
* **Las dos innovaciones de SA-IS:**
  1. **Muestreo de subcadenas LMS (*Leftmost S-type*):** En lugar de seleccionar todas las subcadenas tipo S, SA-IS selecciona únicamente las subcadenas LMS, las cuales forman bloques más largos y reducen más rápido el problema (a lo sumo a $\frac{1}{2}n$ en el peor caso y alrededor de $\frac{1}{3}n$ en promedio).
  2. **Ordenamiento inducido casi puro:** SA-IS aplica el ordenamiento inducido **tanto en el Paso 1** (para ordenar las subcadenas LMS de longitud variable sin usar listas de distancia) **como en el Paso 3** (para propagar el orden de $SA_1$ hacia $SA$), reduciendo el código a menos de 100 líneas en C.

## 2. Definiciones y Lemas Fundamentales (Sección 2.1 y 2.2)
Sea $S[0..n-1]$ una cadena terminada en el centinela único $S[n-1] = \$$, menor que cualquier otro carácter.
* **Clasificación L y S de derecha a izquierda:**
  * El último sufijo $\text{suf}(S, n-1) = \$$ se define siempre como **tipo S**.
  * Para cualquier $i \in [0, n-2]$:
    * $S[i]$ es **tipo S** si $S[i] < S[i+1]$, o si $S[i] = S[i+1]$ y $S[i+1]$ es tipo S.
    * $S[i]$ es **tipo L** si $S[i] > S[i+1]$, o si $S[i] = S[i+1]$ y $S[i+1]$ es tipo L.
  * Con esta propiedad, un solo escaneo de derecha a izquierda llena el arreglo booleano de tipos `t[0..n-1]` en tiempo $\mathcal{O}(n)$.
* **Definición 2.1 (Carácter LMS):** Un carácter $S[i]$ (con $i \in [1, n-1]$) es *leftmost S-type* (LMS) si $S[i]$ es de tipo S y su vecino izquierdo $S[i-1]$ es de tipo L.
* **Definición 2.2 (Subcadena LMS):** Es (i) una subcadena $S[i..j]$ ($i \neq j$) donde tanto $S[i]$ como $S[j]$ son caracteres LMS y no hay ningún otro carácter LMS en medio; o (ii) el propio centinela $\$$.
* **Definición 2.3 (Orden de Subcadenas LMS):** Para comparar dos subcadenas LMS de izquierda a derecha, se compara en cada posición primero el carácter y, si los caracteres son iguales, su tipo, dando **mayor prioridad al tipo S sobre el tipo L** (porque ante el mismo carácter inicial, un sufijo tipo S es lexicográficamente mayor que un sufijo tipo L). Dos subcadenas LMS reciben el mismo nombre entero en $S_1$ si y solo si tienen la misma longitud, los mismos caracteres y los mismos tipos en cada posición.
* **Definición 2.4 (Arreglo de Punteros $P_1$):** Contiene los índices donde inician todas las subcadenas LMS en $S$, en su orden posicional de izquierda a derecha.
* **Los 4 Lemas de la Reducción (Sección 2.2):**
  * **Lema 2.1 (Razón de reducción $1/2$):** $|S_1| \le \lfloor n/2 \rfloor$, dado que $S[0]$ nunca es LMS y nunca existen dos caracteres LMS seguidos.
  * **Lema 2.2 (Centinela en $S_1$):** El último carácter de $S_1$ (que representa al centinela de $S$) es el único carácter más pequeño de $S_1$.
  * **Lema 2.3 (Cobertura):** Si $S_1[i] = S_1[j]$, entonces $P_1[i+1] - P_1[i] = P_1[j+1] - P_1[j]$ (las subcadenas LMS con igual nombre miden exactamente lo mismo).
  * **Lema 2.4 (Preservación de Orden):** El orden relativo de cualesquiera dos sufijos $\text{suf}(S_1, i)$ y $\text{suf}(S_1, j)$ en la cadena reducida $S_1$ coincide exactamente con el orden relativo de los sufijos LMS $\text{suf}(S, P_1[i])$ y $\text{suf}(S, P_1[j])$ en la cadena original $S$.

## 3. El Mecanismo de Cubetas (*Buckets*) y los 3 Pasos de Inducción (Sección 2.3 y 2.4)

### División de Cubetas en `SA`
En el arreglo de sufijos `SA`, todos los sufijos que empiezan con la misma letra $c$ quedan agrupados juntos formando la "cubeta" (*bucket*) de $c$. Además, si dos sufijos empiezan con la misma letra $c$, pero uno es tipo L y otro es tipo S, **el de tipo L siempre es lexicográficamente menor que el de tipo S**. Por eso cada cubeta se divide en dos subcubetas:
* **Mitad izquierda (Subcubeta L):** Para los sufijos tipo L; se llena desde su cabeza (*head*) avanzando hacia la derecha.
* **Mitad derecha (Subcubeta S):** Para los sufijos tipo S; se llena desde su final (*end*) retrocediendo hacia la izquierda.

### Algoritmo de Inducción en 3 Barridos Lineales (Lemas 2.5, 2.6, 2.7 y Teorema 2.1)
Este mismo procedimiento de 3 pasos se ejecuta dos veces en cada nivel de SA-IS: primero en la **Fase de Reducción** (Sección 2.4, para ordenar las subcadenas LMS) y luego en la **Fase de Propagación** (Sección 2.3, para inducir `SA` a partir de `SA1`):
1. **Paso 1 (Colocación de semillas LMS):** Se inicializa todo `SA` en `-1` y se localiza el final (*end*) de cada cubeta S.
   * *Si estamos ordenando subcadenas LMS (antes de la recursión):* Se colocan todos los índices LMS de $P_1$ al final de sus respectivas cubetas S según su primer carácter (sin importar el orden entre los que caen en la misma cubeta).
   * *Si estamos induciendo `SA` desde `SA1` (después de la recursión):* Se colocan los índices LMS (`P1[SA1[i]]`) al final de sus cubetas S respetando estrictamente el orden ya resuelto en `SA1`.
2. **Paso 2 (Barrido de izquierda a derecha para inducir todos los tipo L):** Se posicionan punteros en la cabeza (*head*) de cada cubeta L y se recorre `SA` desde $i = 0$ hasta $n-1$. Por cada valor `SA[i] > 0`, se mira el carácter anterior $j = SA[i] - 1$: si $S[j]$ es de **tipo L**, se inserta $j$ en el *head* actual de la cubeta de $S[j]$ y se avanza ese *head* una posición a la derecha (Lema 2.7).
3. **Paso 3 (Barrido de derecha a izquierda para inducir todos los tipo S):** Se posicionan punteros en el final (*end*) de cada cubeta S y se recorre `SA` desde $i = n-1$ hasta $0$. Por cada valor `SA[i] > 0`, se mira el carácter anterior $j = SA[i] - 1$: si $S[j]$ es de **tipo S**, se inserta $j$ en el *end* actual de la cubeta de $S[j]$ y se retrocede ese *end* una posición a la izquierda (Lema 2.6 y Teorema 2.1, sobrescribiendo las semillas iniciales de tamaño 1).

### Traza del Ejemplo del Paper (Sección 4): `S = mmiississippii$` ($n = 17$)
1. **Cadena y arreglo de tipos `t`:** Para los 17 caracteres (con `$` en el índice 16), el escaneo de derecha a izquierda produce `t = LLSSLLSSLLSSLLLLS`.
2. **Posiciones LMS (donde hay cambio `L -> S`):** Índices `02`, `06`, `10` y `16`.
3. **Paso 1:** En la cubeta `$` entra `[16]`; al final de la cubeta `i` entran `10, 06, 02`.
4. **Paso 2 (Inducción L de izquierda a derecha):** A partir de `16` se induce `15` (tipo L en cubeta `i`), de `15` se induce `14` (cubeta `i`), de `14` se induce `13` (cubeta `p`), ordenando todos los prefijos L.
5. **Paso 3 (Inducción S de derecha a izquierda):** Desde el final de `SA` se inducen los tipos S (`03`, `07`, `11`, `02`, `06`, `10`), dejando todas las subcadenas LMS ordenadas en `SA`.
6. **Nombramiento en $S_1$:** Al recorrer los LMS ordenados (`16, 10, 06, 02`), el centinela `16` recibe el nombre `0`, el LMS `10` recibe `1`, y como las subcadenas LMS que inician en `02` (`issi`) y `06` (`issi`) son idénticas tanto en letras como en tipos, ambas reciben el nombre `2`. Así se obtiene la cadena reducida `S1 = [2, 2, 1, 0]`.

## 4. Complejidad Espacial, Probabilística y Benchmarks (Secciones 3 y 5)
* **Teorema 3.1 (Tiempo y Espacio):** La recurrencia temporal es $\mathcal{T}(n) = \mathcal{T}(\lfloor n/2 \rfloor) + \mathcal{O}(n) = \mathcal{O}(n)$ y la complejidad espacial es $\mathcal{O}(n \lceil \log n \rceil)$ bits.
* **Corolario 3.1 (Espacio de trabajo en el peor caso):** Si se mantienen los arreglos de tipos `t` en cada nivel recursivo, ocupan a lo sumo $2n$ bits en total ($0.25$ bytes/carácter), o $n$ bits si se liberan y recalculan al volver de la recursión. El espacio adicional de trabajo en el peor caso es $0.5n \log n + n + \mathcal{O}(1)$ bits para alfabeto constante y $n \log n + n + \mathcal{O}(1)$ bits para alfabeto entero.
* **Teorema 3.2 (Reducción media en la práctica):** Si cada carácter tiene probabilidad independiente $1/2$ de ser tipo L o S, la longitud esperada de una subcadena LMS sin contar su extremo compartido es $3$, lo que arroja una **razón de reducción promedio $\le 1/3$**.
* **Resultados Experimentales del Paper (Tabla 1 - Corpus Manzini-Ferragina, 300 MB totales):**
  * Evaluado sobre 7 datasets (`world`, `bible`, `chr22`, `E.coli`, `sprot`, `etext`, `howto`) compilando en `g++ -O3`.
  * **Velocidad:** SA-IS promedia **0.86 segundos/MB** frente a 1.23 s/MB del algoritmo KA (KA toma 43% más tiempo).
  * **Memoria Pico (*Heap Peak*):** SA-IS consume en promedio **5.12 bytes por carácter** (frente a 8.51 B/char de KA), prácticamente alcanzando el límite teórico mínimo de $5n$ bytes ($4n$ bytes del arreglo `SA` de 32 bits + $1n$ byte del texto original `S`).
  * **Profundidad y Ratio de Reducción:** SA-IS logra un ratio de reducción empírico promedio de **0.32** (confirmando el Teorema 3.2) y una profundidad media de **7.43 niveles recursivos**.