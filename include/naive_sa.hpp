#ifndef NAIVE_SA_HPP
#define NAIVE_SA_HPP

#include <algorithm>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

namespace eda {

/**
 * @brief Construye el Arreglo de Sufijos (Suffix Array) de manera ingenua (Naive).
 *
 * Complejidad Temporal: O(n^2 log n) en el peor caso, ya que std::sort realiza
 * O(n log n) comparaciones y cada comparacion lexicografica entre dos sufijos
 * puede recorrer hasta O(n) caracteres.
 *
 * Complejidad Espacial: O(n) adicional al arreglo de salida.
 * En lugar de generar subcadenas con std::string::substr() (lo cual consumiria
 * O(n^2) de memoria RAM), se ordenan directamente los indices [0, n-1] y se
 * comparan vistas ligeras (puntero + longitud) mediante std::string_view sobre
 * el buffer original contiguo en memoria.
 *
 * @param s Cadena de entrada de longitud n.
 * @return std::vector<int> Arreglo SA de tamano n con los indices iniciales
 *         de los sufijos ordenados lexicograficamente.
 */
inline std::vector<int> build_naive_sa(const std::string& s) {
    const int n = static_cast<int>(s.size());
    std::vector<int> sa(n);

    // Inicializamos SA con las posiciones originales: [0, 1, 2, ..., n-1]
    std::iota(sa.begin(), sa.end(), 0);

    // Vista sin copia sobre el buffer de caracteres de la cadena original
    const std::string_view sv(s);

    // Ordenamos los indices comparando lexicograficamente los sufijos S[a..n-1] y S[b..n-1]
    // Nota: en C++ char puede tener signo (-128 a 127). Para ser 100% consistentes
    // con el alfabeto entero sin signo [0..255] (mapeado a [1..256] en DC3 y SA-IS),
    // comparamos byte a byte como unsigned char.
    std::sort(sa.begin(), sa.end(), [&sv, n](int a, int b) {
        if (a == b) return false;
        const int len_a = n - a;
        const int len_b = n - b;
        const int min_len = std::min(len_a, len_b);

        for (int k = 0; k < min_len; ++k) {
            const auto ca = static_cast<unsigned char>(sv[a + k]);
            const auto cb = static_cast<unsigned char>(sv[b + k]);
            if (ca != cb) {
                return ca < cb;
            }
        }
        // Si todos los caracteres en el prefijo comun coinciden, el sufijo mas corto
        // es menor (equivale a encontrarse primero con el caracter centinela '$' = 0).
        return len_a < len_b;
    });

    return sa;
}

/**
 * @brief Convierte un std::string en un arreglo de enteros con centinela explicito '$' = 0.
 *
 * Esta funcion ilustra la transformacion fundamental requerida por los papers:
 * - Cada byte c se convierte primero a unsigned char (0..255) y se desplaza en +1
 *   al rango [1, 256] para soportar cadenas binarias con bytes nulos '\0'.
 * - Se agrega un 0 al final en la posicion n que actua como centinela '$'
 *   (estrictamente menor que cualquier caracter del alfabeto desplazado [1, 256]).
 *
 * @param s Cadena de entrada de longitud n.
 * @return std::vector<int> Arreglo de tamano n + 1 terminado en 0.
 */
inline std::vector<int> string_to_int_with_sentinel(const std::string& s) {
    const int n = static_cast<int>(s.size());
    std::vector<int> T(n + 1);
    for (int i = 0; i < n; ++i) {
        T[i] = static_cast<int>(static_cast<unsigned char>(s[i])) + 1;
    }
    T[n] = 0; // Centinela '$'
    return T;
}

/**
 * @brief Verifica de forma independiente si un Suffix Array es valido para la cadena s.
 *
 * Comprueba dos invariantes:
 * 1. Que SA sea una permutacion exacta del conjunto {0, 1, ..., n-1}.
 * 2. Que los sufijos esten en orden lexicografico estrictamente creciente:
 *    S[SA[0]..n-1] < S[SA[1]..n-1] < ... < S[SA[n-1]..n-1].
 */
inline bool verify_suffix_array(const std::string& s, const std::vector<int>& sa) {
    const int n = static_cast<int>(s.size());
    if (static_cast<int>(sa.size()) != n) {
        return false;
    }

    std::vector<bool> seen(n, false);
    for (int idx : sa) {
        if (idx < 0 || idx >= n || seen[idx]) {
            return false; // Indice fuera de rango o duplicado
        }
        seen[idx] = true;
    }

    for (int i = 0; i + 1 < n; ++i) {
        const int a = sa[i];
        const int b = sa[i + 1];
        const int min_len = std::min(n - a, n - b);
        bool strictly_less = false;
        bool decided = false;

        for (int k = 0; k < min_len; ++k) {
            const auto ca = static_cast<unsigned char>(s[a + k]);
            const auto cb = static_cast<unsigned char>(s[b + k]);
            if (ca != cb) {
                strictly_less = (ca < cb);
                decided = true;
                break;
            }
        }
        if (!decided) {
            // Ningun par de sufijos distintos tiene la misma longitud, asi que el mas corto es menor
            strictly_less = (n - a) < (n - b);
        }
        if (!strictly_less) {
            return false;
        }
    }

    return true;
}

}

#endif // NAIVE_SA_HPP
