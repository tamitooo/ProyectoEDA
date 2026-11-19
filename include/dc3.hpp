#ifndef DC3_HPP
#define DC3_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace eda {

/**
 * @brief Estructura de metricas recolectadas durante la ejecucion de DC3.
 */
struct DC3Stats {
    double elapsed_ms = 0.0;                  // Tiempo total wall-clock en milisegundos
    int max_recursion_depth = 0;              // Profundidad maxima de llamadas recursivas
    double first_level_reduction_ratio = 0.0; // Razon n02 / n en el nivel superior (~0.6667)
    std::size_t peak_aux_bytes = 0;           // Pico de memoria auxiliar en el heap (s12, SA12, s0, SA0, c)
    std::size_t total_peak_bytes = 0;         // Pico total incluyendo los arreglos base s y SA
    double bytes_per_char = 0.0;              // total_peak_bytes / n
};

// Compara lexicograficamente dos pares de enteros (a1, a2) <= (b1, b2) en O(1).
inline bool leq(int a1, int a2, int b1, int b2) {
    return (a1 < b1 || (a1 == b1 && a2 <= b2));
}

// Compara lexicograficamente dos tripletas (a1, a2, a3) <= (b1, b2, b3) en O(1).
inline bool leq(int a1, int a2, int a3, int b1, int b2, int b3) {
    return (a1 < b1 || (a1 == b1 && leq(a2, a3, b2, b3)));
}

// Ordena de forma estable los indices en a[0..n-1] hacia b[0..n-1] usando claves enteras en el rango [0..K] extraidas de r.
void radixPass(const int* a, int* b, const int* r, int n, int K);

// Ejecuta la Fase 1 del algoritmo DC3 (Apendice A de Karkkainen & Sanders, 2003).
int dc3Phase1(
    const int* s,
    int n,
    int K,
    std::vector<int>& s12,
    std::vector<int>& SA12,
    bool recurse_on_ties = false
);

// Nucleo recursivo del algoritmo DC3 / Skew (Apendice A de Karkkainen & Sanders, 2003).
void suffixArray(const int* s, int* SA, int n, int K);

/**
 * @brief Envoltorio publico para construir el Suffix Array de un std::string mediante DC3.
 *
 * @param str Cadena de entrada de longitud n.
 * @param stats Puntero opcional para recolectar metricas de tiempo, memoria y recursion.
 * @return std::vector<int> Arreglo de sufijos SA de tamano n.
 */
std::vector<int> build_dc3_sa(const std::string& str, DC3Stats* stats = nullptr);

}

#endif // DC3_HPP
