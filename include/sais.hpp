#ifndef SAIS_HPP
#define SAIS_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace eda {

/**
 * @brief Constantes para los tipos de sufijos en el algoritmo SA-IS (Nong et al., 2009).
 */
constexpr int L_TYPE = 0;
constexpr int S_TYPE = 1;

/**
 * @brief Estructura de metricas recolectadas durante la ejecucion de SA-IS.
 */
struct SAISStats {
    double elapsed_ms = 0.0;                  // Tiempo total wall-clock en milisegundos
    int max_recursion_depth = 0;              // Profundidad maxima de llamadas recursivas
    double first_level_reduction_ratio = 0.0; // Razon n1 / n en el nivel superior (<= 0.5, ~0.33 tipico)
    std::size_t peak_aux_bytes = 0;           // Pico de memoria auxiliar en el heap (t y bkt; SA1 y s1 usan SA)
    std::size_t total_peak_bytes = 0;         // Pico total incluyendo los arreglos base s y SA
    double bytes_per_char = 0.0;              // total_peak_bytes / n
    double compact_bytes_per_char = 0.0;      // Modelo compacto del paper (s en 1B, SA en 4B, t en 1 bit)
};

inline bool isLMS(int i, const int* t) {
    return i > 0 && t[i] == S_TYPE && t[i - 1] == L_TYPE;
}

/**
 * @brief Compara dos subcadenas LMS que inician en pos1 y pos2 (Definicion 2.3 de Nong et al.).
 */
inline bool lmsSubstringsAreEqual(const int* s, const int* t, int n, int pos1, int pos2) {
    if (pos1 < 0 || pos2 < 0) {
        return false;
    }
    for (int d = 0; d <= n; ++d) {
        if (s[pos1 + d] != s[pos2 + d] || t[pos1 + d] != t[pos2 + d]) {
            return false;
        }
        if (d > 0 && (isLMS(pos1 + d, t) || isLMS(pos2 + d, t))) {
            return true;
        }
    }
    return true;
}

/**
 * @brief Clasifica cada posicion de s[0..n] en L_TYPE (0) o S_TYPE (1) en tiempo O(n).
 */
void classifyLS(const int* s, int* t, int n);

/**
 * @brief Calcula los limites iniciales (cabezas) o finales (colas) de cada cubeta (bucket).
 */
void getBuckets(const int* s, int* bkt, int n, int K, bool end);

/**
 * @brief Paso 2 de la induccion: barre SA[0..n] de izquierda a derecha para inducir
 *        el orden de todos los sufijos de tipo L (L_TYPE).
 */
void induceSAl(const int* t, int* SA, const int* s, int* bkt, int n, int K);

/**
 * @brief Paso 3 de la induccion: barre SA[0..n] de derecha a izquierda para inducir
 *        el orden de todos los sufijos de tipo S (S_TYPE).
 */
void induceSAs(const int* t, int* SA, const int* s, int* bkt, int n, int K);

/**
 * @brief Nucleo recursivo del algoritmo SA-IS (Nong, Zhang & Chan, 2009).
 */
void sais_core(const int* s, int* SA, int n, int K);

/**
 * @brief Envoltorio publico para construir el Suffix Array de un std::string mediante SA-IS.
 *
 * @param str Cadena de entrada de longitud n.
 * @param stats Puntero opcional para recolectar metricas de tiempo, memoria y recursion.
 * @return std::vector<int> Arreglo de sufijos SA de tamano n.
 */
std::vector<int> build_sais_sa(const std::string& str, SAISStats* stats = nullptr);

} // namespace eda

#endif // SAIS_HPP
