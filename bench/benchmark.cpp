#include "dc3.hpp"
#include "naive_sa.hpp"
#include "sais.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif

namespace {

/**
 * @brief Obtiene el consumo pico de memoria fisica del proceso (Peak Working Set / RSS) en bytes.
 */
std::size_t get_os_peak_rss_bytes() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<std::size_t>(pmc.PeakWorkingSetSize);
    }
#endif
    return 0;
}

/**
 * @brief Simulador de jerarquia de cache L1D (32 KB, 8-way, 64B line) y L2 (256 KB, 8-way, 64B line)
 *        con politica de reemplazo LRU y clasificacion de accesos secuenciales vs. indirectos.
 */
class CacheHierarchySimulator {
public:
    static constexpr std::size_t LINE_BYTES = 64; // 16 ints de 4 bytes por linea de cache
    static constexpr std::size_t L1_SETS = 64;    // 64 sets * 8 ways * 64B = 32 KB L1D
    static constexpr std::size_t L1_WAYS = 8;
    static constexpr std::size_t L2_SETS = 512;   // 512 sets * 8 ways * 64B = 256 KB L2
    static constexpr std::size_t L2_WAYS = 8;

    std::uint64_t total_accesses = 0;
    std::uint64_t sequential_accesses = 0;
    std::uint64_t indirect_accesses = 0;
    std::uint64_t l1_hits = 0;
    std::uint64_t l1_misses = 0;
    std::uint64_t l2_hits = 0;
    std::uint64_t l2_misses = 0;

    CacheHierarchySimulator() {
        reset();
    }

    void reset() {
        total_accesses = 0;
        sequential_accesses = 0;
        indirect_accesses = 0;
        l1_hits = 0;
        l1_misses = 0;
        l2_hits = 0;
        l2_misses = 0;
        tick_ = 0;
        for (auto& set : l1_tags_) set.fill(UINT64_MAX);
        for (auto& set : l1_lru_)  set.fill(0);
        for (auto& set : l2_tags_) set.fill(UINT64_MAX);
        for (auto& set : l2_lru_)  set.fill(0);
    }

    inline void access(const void* addr, bool is_sequential) {
        total_accesses++;
        if (is_sequential) {
            sequential_accesses++;
        } else {
            indirect_accesses++;
        }

        const std::uint64_t line_addr = reinterpret_cast<std::uintptr_t>(addr) / LINE_BYTES;
        tick_++;

        // 1. Consulta en L1D
        const std::size_t l1_set = static_cast<std::size_t>(line_addr % L1_SETS);
        for (std::size_t w = 0; w < L1_WAYS; ++w) {
            if (l1_tags_[l1_set][w] == line_addr) {
                l1_hits++;
                l1_lru_[l1_set][w] = tick_;
                return;
            }
        }

        // Fallo en L1D -> insertar en L1D (reemplazo LRU) y consultar L2
        l1_misses++;
        std::size_t l1_victim = 0;
        for (std::size_t w = 1; w < L1_WAYS; ++w) {
            if (l1_lru_[l1_set][w] < l1_lru_[l1_set][l1_victim]) {
                l1_victim = w;
            }
        }
        l1_tags_[l1_set][l1_victim] = line_addr;
        l1_lru_[l1_set][l1_victim] = tick_;

        // 2. Consulta en L2
        const std::size_t l2_set = static_cast<std::size_t>(line_addr % L2_SETS);
        for (std::size_t w = 0; w < L2_WAYS; ++w) {
            if (l2_tags_[l2_set][w] == line_addr) {
                l2_hits++;
                l2_lru_[l2_set][w] = tick_;
                return;
            }
        }

        // Fallo en L2 -> insertar en L2 (reemplazo LRU)
        l2_misses++;
        std::size_t l2_victim = 0;
        for (std::size_t w = 1; w < L2_WAYS; ++w) {
            if (l2_lru_[l2_set][w] < l2_lru_[l2_set][l2_victim]) {
                l2_victim = w;
            }
        }
        l2_tags_[l2_set][l2_victim] = line_addr;
        l2_lru_[l2_set][l2_victim] = tick_;
    }

private:
    std::uint64_t tick_ = 0;
    std::array<std::array<std::uint64_t, L1_WAYS>, L1_SETS> l1_tags_{};
    std::array<std::array<std::uint64_t, L1_WAYS>, L1_SETS> l1_lru_{};
    std::array<std::array<std::uint64_t, L2_WAYS>, L2_SETS> l2_tags_{};
    std::array<std::array<std::uint64_t, L2_WAYS>, L2_SETS> l2_lru_{};
};

// Instrumentacion de DC3 sobre el simulador de cache
void sim_dc3_radixPass(
    const int* a, int* b, const int* r, int n, int K, CacheHierarchySimulator& sim
) {
    std::vector<int> c(K + 1, 0);
    for (int i = 0; i <= K; ++i) {
        sim.access(&c[i], true);
    }
    for (int i = 0; i < n; ++i) {
        sim.access(&a[i], true);          // Lectura secuencial de a[i]
        int idx = a[i];
        sim.access(&r[idx], false);       // Salto indirecto a r[a[i]]
        int key = r[idx];
        sim.access(&c[key], false);       // Incremento indirecto en cubeta c[key]
        c[key]++;
    }
    for (int i = 0, sum = 0; i <= K; ++i) {
        sim.access(&c[i], true);
        int t = c[i];
        c[i] = sum;
        sum += t;
    }
    for (int i = 0; i < n; ++i) {
        sim.access(&a[i], true);          // Lectura secuencial de a[i]
        int idx = a[i];
        sim.access(&r[idx], false);       // Salto indirecto a r[a[i]]
        int key = r[idx];
        sim.access(&c[key], false);       // Consulta/post-incremento de puntero de cubeta
        int dst = c[key]++;
        sim.access(&b[dst], false);       // Escritura dispersa en la cubeta destino de b[]
        b[dst] = idx;
    }
}

void sim_dc3_suffixArray(const int* s, int* SA, int n, int K, CacheHierarchySimulator& sim) {
    if (n <= 0) return;
    if (n == 1) {
        sim.access(&SA[0], true);
        SA[0] = 0;
        return;
    }
    int n0 = (n + 2) / 3;
    int n1 = (n + 1) / 3;
    int n2 = n / 3;
    int n02 = n0 + n2;

    std::vector<int> s12(n02 + 3, 0);
    std::vector<int> SA12(n02 + 3, 0);

    for (int i = 0, j = 0; i < n + (n0 - n1); ++i) {
        if (i % 3 != 0) {
            sim.access(&s12[j], true);
            s12[j++] = i;
        }
    }

    sim_dc3_radixPass(s12.data(), SA12.data(), s + 2, n02, K, sim);
    sim_dc3_radixPass(SA12.data(), s12.data(), s + 1, n02, K, sim);
    sim_dc3_radixPass(s12.data(), SA12.data(), s,     n02, K, sim);

    int name = 0, c0 = -1, c1 = -1, c2 = -1;
    for (int i = 0; i < n02; ++i) {
        sim.access(&SA12[i], true);
        int pos = SA12[i];
        sim.access(&s[pos], false);
        sim.access(&s[pos + 1], true);
        sim.access(&s[pos + 2], true);
        if (s[pos] != c0 || s[pos + 1] != c1 || s[pos + 2] != c2) {
            name++;
            c0 = s[pos];
            c1 = s[pos + 1];
            c2 = s[pos + 2];
        }
        int target = (pos % 3 == 1) ? (pos / 3) : (pos / 3 + n0);
        sim.access(&s12[target], false);
        s12[target] = name;
    }

    if (name < n02) {
        sim_dc3_suffixArray(s12.data(), SA12.data(), n02, name, sim);
        for (int i = 0; i < n02; ++i) {
            sim.access(&SA12[i], true);
            sim.access(&s12[SA12[i]], false);
            s12[SA12[i]] = i + 1;
        }
    } else {
        for (int i = 0; i < n02; ++i) {
            sim.access(&s12[i], true);
            sim.access(&SA12[s12[i] - 1], false);
            SA12[s12[i] - 1] = i;
        }
    }

    std::vector<int> s0(n0, 0);
    std::vector<int> SA0(n0, 0);
    for (int i = 0, j = 0; i < n02; ++i) {
        sim.access(&SA12[i], true);
        if (SA12[i] < n0) {
            sim.access(&s0[j], true);
            s0[j++] = 3 * SA12[i];
        }
    }
    sim_dc3_radixPass(s0.data(), SA0.data(), s, n0, K, sim);

    for (int p = 0, t = n0 - n1, k = 0; k < n; ++k) {
        sim.access(&SA12[t], true);
        sim.access(&SA0[p], true);
        int i = (SA12[t] < n0) ? (SA12[t] * 3 + 1) : ((SA12[t] - n0) * 3 + 2);
        int j = SA0[p];

        sim.access(&s[i], false);
        sim.access(&s[j], false);
        bool sa12_smaller = false;
        if (SA12[t] < n0) {
            sim.access(&s12[SA12[t] + n0], false);
            sim.access(&s12[j / 3], false);
            sa12_smaller = eda::leq(s[i], s12[SA12[t] + n0], s[j], s12[j / 3]);
        } else {
            sim.access(&s[i + 1], true);
            sim.access(&s[j + 1], true);
            sim.access(&s12[SA12[t] - n0 + 1], false);
            sim.access(&s12[j / 3 + n0], false);
            sa12_smaller = eda::leq(s[i], s[i + 1], s12[SA12[t] - n0 + 1], s[j], s[j + 1], s12[j / 3 + n0]);
        }

        sim.access(&SA[k], true);
        if (sa12_smaller) {
            SA[k] = i;
            t++;
            if (t == n02) {
                for (k++; p < n0; p++, k++) {
                    sim.access(&SA0[p], true);
                    sim.access(&SA[k], true);
                    SA[k] = SA0[p];
                }
            }
        } else {
            SA[k] = j;
            p++;
            if (p == n0) {
                for (k++; t < n02; t++, k++) {
                    sim.access(&SA12[t], true);
                    sim.access(&SA[k], true);
                    SA[k] = (SA12[t] < n0) ? (SA12[t] * 3 + 1) : ((SA12[t] - n0) * 3 + 2);
                }
            }
        }
    }
}

// Instrumentacion de SA-IS sobre el simulador de cache
void sim_sais_getBuckets(const int* s, int* bkt, int n, int K, bool end, CacheHierarchySimulator& sim) {
    for (int i = 0; i <= K; ++i) {
        sim.access(&bkt[i], true);
        bkt[i] = 0;
    }
    for (int i = 0; i <= n; ++i) {
        sim.access(&s[i], true);
        sim.access(&bkt[s[i]], false);
        bkt[s[i]]++;
    }
    int sum = 0;
    for (int i = 0; i <= K; ++i) {
        sim.access(&bkt[i], true);
        sum += bkt[i];
        bkt[i] = end ? (sum - 1) : (sum - bkt[i]);
    }
}

void sim_sais_induceSAl(const int* t, int* SA, const int* s, int* bkt, int n, int K, CacheHierarchySimulator& sim) {
    sim_sais_getBuckets(s, bkt, n, K, false, sim);
    for (int i = 0; i <= n; ++i) {
        sim.access(&SA[i], true);
        int j = SA[i] - 1;
        if (j >= 0) {
            sim.access(&t[j], false);
            if (t[j] == eda::L_TYPE) {
                sim.access(&s[j], false);
                int c = s[j];
                sim.access(&bkt[c], false);
                int dst = bkt[c]++;
                sim.access(&SA[dst], true); // Avance contiguo dentro de la subcubeta L
                SA[dst] = j;
            }
        }
    }
}

void sim_sais_induceSAs(const int* t, int* SA, const int* s, int* bkt, int n, int K, CacheHierarchySimulator& sim) {
    sim_sais_getBuckets(s, bkt, n, K, true, sim);
    for (int i = n; i >= 0; --i) {
        sim.access(&SA[i], true);
        int j = SA[i] - 1;
        if (j >= 0) {
            sim.access(&t[j], false);
            if (t[j] == eda::S_TYPE) {
                sim.access(&s[j], false);
                int c = s[j];
                sim.access(&bkt[c], false);
                int dst = bkt[c]--;
                sim.access(&SA[dst], true); // Retroceso contiguo dentro de la subcubeta S
                SA[dst] = j;
            }
        }
    }
}

void sim_sais_core(const int* s, int* SA, int n, int K, CacheHierarchySimulator& sim) {
    std::vector<int> t(n + 1, 0);
    sim.access(&t[n], true);
    t[n] = eda::S_TYPE;
    for (int i = n - 1; i >= 0; --i) {
        sim.access(&s[i], true);
        sim.access(&s[i + 1], true);
        sim.access(&t[i], true);
        t[i] = (s[i] < s[i + 1] || (s[i] == s[i + 1] && t[i + 1] == eda::S_TYPE)) ? eda::S_TYPE : eda::L_TYPE;
    }

    std::vector<int> bkt(K + 1, 0);
    sim_sais_getBuckets(s, bkt.data(), n, K, true, sim);
    for (int i = 0; i <= n; ++i) {
        sim.access(&SA[i], true);
        SA[i] = -1;
    }
    for (int i = 1; i <= n; ++i) {
        sim.access(&t[i], true);
        if (eda::isLMS(i, t.data())) {
            sim.access(&s[i], true);
            int dst = bkt[s[i]]--;
            sim.access(&SA[dst], true);
            SA[dst] = i;
        }
    }
    sim_sais_induceSAl(t.data(), SA, s, bkt.data(), n, K, sim);
    sim_sais_induceSAs(t.data(), SA, s, bkt.data(), n, K, sim);

    int n1 = 0;
    for (int i = 0; i <= n; ++i) {
        sim.access(&SA[i], true);
        if (eda::isLMS(SA[i], t.data())) {
            sim.access(&SA[n1], true);
            SA[n1++] = SA[i];
        }
    }

    for (int i = n1; i <= n; ++i) {
        sim.access(&SA[i], true);
        SA[i] = -1;
    }
    int name = 0, prev = -1;
    for (int i = 0; i < n1; ++i) {
        sim.access(&SA[i], true);
        int pos = SA[i];
        bool equal = false;
        if (prev >= 0) {
            for (int d = 0; d <= n; ++d) {
                sim.access(&s[pos + d], d > 0);
                sim.access(&s[prev + d], d > 0);
                sim.access(&t[pos + d], d > 0);
                sim.access(&t[prev + d], d > 0);
                if (s[pos + d] != s[prev + d] || t[pos + d] != t[prev + d]) {
                    equal = false;
                    break;
                }
                if (d > 0 && (eda::isLMS(pos + d, t.data()) || eda::isLMS(prev + d, t.data()))) {
                    equal = true;
                    break;
                }
            }
        }
        if (!equal) {
            name++;
            prev = pos;
        }
        sim.access(&SA[n1 + pos / 2], false);
        SA[n1 + pos / 2] = name - 1;
    }

    for (int i = n, j = n; i >= n1; --i) {
        sim.access(&SA[i], true);
        if (SA[i] >= 0) {
            sim.access(&SA[j], true);
            SA[j--] = SA[i];
        }
    }

    int* SA1 = SA;
    int* s1 = SA + n + 1 - n1;
    if (name < n1) {
        sim_sais_core(s1, SA1, n1 - 1, name - 1, sim);
    } else {
        for (int i = 0; i < n1; ++i) {
            sim.access(&s1[i], true);
            sim.access(&SA1[s1[i]], false);
            SA1[s1[i]] = i;
        }
    }

    sim_sais_getBuckets(s, bkt.data(), n, K, true, sim);
    for (int i = 1, j = 0; i <= n; ++i) {
        sim.access(&t[i], true);
        if (eda::isLMS(i, t.data())) {
            sim.access(&s1[j], true);
            s1[j++] = i;
        }
    }
    for (int i = 0; i < n1; ++i) {
        sim.access(&SA1[i], true);
        sim.access(&s1[SA1[i]], false);
        SA1[i] = s1[SA1[i]];
    }
    for (int i = n1; i <= n; ++i) {
        sim.access(&SA[i], true);
        SA[i] = -1;
    }
    for (int i = n1 - 1; i >= 0; --i) {
        sim.access(&SA[i], true);
        int j = SA[i];
        SA[i] = -1;
        sim.access(&s[j], false);
        int dst = bkt[s[j]]--;
        sim.access(&SA[dst], true);
        SA[dst] = j;
    }
    sim_sais_induceSAl(t.data(), SA, s, bkt.data(), n, K, sim);
    sim_sais_induceSAs(t.data(), SA, s, bkt.data(), n, K, sim);
}

/**
 * @brief Genera una cadena aleatoria uniforme sobre un alfabeto de tamano sigma_size.
 */
std::string generate_random_text(int n, int sigma_size, unsigned int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, sigma_size - 1);
    std::string s(n, ' ');
    for (int i = 0; i < n; ++i) {
        s[i] = static_cast<char>('!' + dist(rng));
    }
    return s;
}

/**
 * @brief Genera una secuencia genomica sintetica sobre el alfabeto {A, C, G, T} con repeticiones.
 */
std::string generate_dna_sequence(int n, unsigned int seed = 1337) {
    static const char bases[4] = {'A', 'C', 'G', 'T'};
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> base_dist(0, 3);
    std::string s;
    s.reserve(n);

    const std::string motif = "GATTACAGATTACATGCA";
    while (static_cast<int>(s.size()) < n) {
        if (rng() % 7 == 0 && static_cast<int>(s.size() + motif.size()) <= n) {
            s += motif;
        } else {
            s.push_back(bases[base_dist(rng)]);
        }
    }
    s.resize(n);
    return s;
}

/**
 * @brief Genera un flujo sintetico de codigo fuente C++ (orientado a analizadores lexicos LR(1)).
 */
std::string generate_source_code(int n, unsigned int seed = 2026) {
    static const std::vector<std::string> tokens = {
        "int ", "void ", "for(int i=0;i<n;++i){", "if(s[i]<s[i+1]){",
        "return ", "std::vector<int> ", "SA[bkt[s[j]]--]=j;", "radixPass(s12,SA12,s,n02,K);",
        "else{", "} ", "while(k<n)", "const char* ", "auto ", "0;\n"
    };
    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::size_t> dist(0, tokens.size() - 1);
    std::string s;
    s.reserve(n + 64);
    while (static_cast<int>(s.size()) < n) {
        s += tokens[dist(rng)];
    }
    s.resize(n);
    return s;
}

/**
 * @brief Genera una cadena altamente periodica para estresar la profundidad recursiva.
 */
std::string generate_periodic_string(int n) {
    const std::string pattern = "mississippi_mmiississippii_abracadabra_";
    std::string s;
    s.reserve(n + pattern.size());
    while (static_cast<int>(s.size()) < n) {
        s += pattern;
    }
    s.resize(n);
    return s;
}

struct BenchmarkRow {
    std::string dataset;
    int n = 0;
    double naive_ms = -1.0;
    double dc3_median_ms = 0.0;
    double dc3_mean_ms = 0.0;
    double sais_median_ms = 0.0;
    double sais_mean_ms = 0.0;
    double speedup_dc3_over_sais = 0.0;
    int dc3_max_depth = 0;
    int sais_max_depth = 0;
    double dc3_reduction_ratio = 0.0;
    double sais_reduction_ratio = 0.0;
    std::size_t dc3_peak_aux_kb = 0;
    std::size_t sais_peak_aux_kb = 0;
    double dc3_bytes_per_char = 0.0;
    double sais_bytes_per_char = 0.0;
    double sais_compact_bpc = 0.0;
};

double compute_median(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const std::size_t m = v.size() / 2;
    if (v.size() % 2 == 1) {
        return v[m];
    }
    return 0.5 * (v[m - 1] + v[m]);
}

double compute_mean(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    const double sum = std::accumulate(v.begin(), v.end(), 0.0);
    return sum / static_cast<double>(v.size());
}

BenchmarkRow run_single_benchmark(
    const std::string& dataset_name,
    const std::string& text,
    int repetitions,
    int max_naive_n
) {
    const int n = static_cast<int>(text.size());
    BenchmarkRow row;
    row.dataset = dataset_name;
    row.n = n;

    eda::DC3Stats dc3_stats{};
    eda::SAISStats sais_stats{};
    const std::vector<int> sa_dc3 = eda::build_dc3_sa(text, &dc3_stats);
    const std::vector<int> sa_sais = eda::build_sais_sa(text, &sais_stats);
    assert(sa_dc3 == sa_sais && "Error critico en benchmark: DC3 y SA-IS difieren.");

    if (n <= max_naive_n) {
        const auto t0 = std::chrono::steady_clock::now();
        const std::vector<int> sa_naive = eda::build_naive_sa(text);
        const auto t1 = std::chrono::steady_clock::now();
        row.naive_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        assert(sa_sais == sa_naive && "Error critico en benchmark: SA-IS y Naive difieren.");
    }

    std::vector<double> dc3_times;
    std::vector<double> sais_times;
    dc3_times.reserve(repetitions);
    sais_times.reserve(repetitions);

    for (int r = 0; r < repetitions; ++r) {
        eda::DC3Stats st_dc3{};
        auto out_dc3 = eda::build_dc3_sa(text, &st_dc3);
        (void)out_dc3;
        dc3_times.push_back(st_dc3.elapsed_ms);
        dc3_stats = st_dc3;

        eda::SAISStats st_sais{};
        auto out_sais = eda::build_sais_sa(text, &st_sais);
        (void)out_sais;
        sais_times.push_back(st_sais.elapsed_ms);
        sais_stats = st_sais;
    }

    row.dc3_median_ms = compute_median(dc3_times);
    row.dc3_mean_ms = compute_mean(dc3_times);
    row.sais_median_ms = compute_median(sais_times);
    row.sais_mean_ms = compute_mean(sais_times);
    row.speedup_dc3_over_sais = (row.sais_median_ms > 0.0) ? (row.dc3_median_ms / row.sais_median_ms) : 0.0;

    row.dc3_max_depth = dc3_stats.max_recursion_depth;
    row.sais_max_depth = sais_stats.max_recursion_depth;
    row.dc3_reduction_ratio = dc3_stats.first_level_reduction_ratio;
    row.sais_reduction_ratio = sais_stats.first_level_reduction_ratio;

    row.dc3_peak_aux_kb = dc3_stats.peak_aux_bytes / 1024;
    row.sais_peak_aux_kb = sais_stats.peak_aux_bytes / 1024;
    row.dc3_bytes_per_char = dc3_stats.bytes_per_char;
    row.sais_bytes_per_char = sais_stats.bytes_per_char;
    row.sais_compact_bpc = sais_stats.compact_bytes_per_char;

    return row;
}

void write_benchmark_csv(const std::string& csv_path, const std::vector<BenchmarkRow>& results) {
    std::ofstream csv(csv_path);
    if (!csv) return;
    csv << "dataset,n,naive_ms,dc3_median_ms,dc3_mean_ms,sais_median_ms,sais_mean_ms,"
           "speedup_dc3_over_sais,dc3_max_depth,sais_max_depth,dc3_reduction_ratio,"
           "sais_reduction_ratio,dc3_peak_aux_kb,sais_peak_aux_kb,dc3_bytes_per_char,"
           "sais_bytes_per_char,sais_compact_bpc\n";
    for (const auto& r : results) {
        csv << r.dataset << ","
            << r.n << ","
            << std::fixed << std::setprecision(4)
            << r.naive_ms << ","
            << r.dc3_median_ms << ","
            << r.dc3_mean_ms << ","
            << r.sais_median_ms << ","
            << r.sais_mean_ms << ","
            << r.speedup_dc3_over_sais << ","
            << r.dc3_max_depth << ","
            << r.sais_max_depth << ","
            << r.dc3_reduction_ratio << ","
            << r.sais_reduction_ratio << ","
            << r.dc3_peak_aux_kb << ","
            << r.sais_peak_aux_kb << ","
            << r.dc3_bytes_per_char << ","
            << r.sais_bytes_per_char << ","
            << r.sais_compact_bpc << "\n";
    }
}

int run_cache_profiler_mode(const std::string& csv_path) {
    std::cout << "========================================================================================================\n";
    std::cout << " PERFILADOR DE JERARQUIA DE CACHE L1D (32KB) / L2 (256KB): DC3 vs SA-IS\n";
    std::cout << "========================================================================================================\n\n";

    struct CaseSpec {
        std::string name;
        int kind;
    };
    const std::vector<CaseSpec> families = {
        {"random_ascii", 0},
        {"dna_genome",   1},
        {"source_code",  2},
        {"periodic_str", 3}
    };
    const std::vector<int> cache_sizes = {10000, 50000, 100000, 200000};

    std::ofstream csv(csv_path);
    csv << "dataset,n,dc3_total_accesses,dc3_indirect_pct,dc3_l1_misses,dc3_l1_miss_rate_pct,"
           "dc3_l2_misses,dc3_l1_misses_per_char,sais_total_accesses,sais_indirect_pct,"
           "sais_l1_misses,sais_l1_miss_rate_pct,sais_l2_misses,sais_l1_misses_per_char,"
           "l1_miss_ratio_dc3_over_sais,l2_miss_ratio_dc3_over_sais\n";

    std::cout << "Familia       |       n | DC3 L1 Miss/ch (Rate%) | SAIS L1 Miss/ch (Rate%) | DC3/SAIS L1 Miss | DC3/SAIS L2 Miss\n";
    std::cout << "-----------------------------------------------------------------------------------------------------------------\n";

    for (const auto& fam : families) {
        for (int n : cache_sizes) {
            std::string text;
            if (fam.kind == 0) text = generate_random_text(n, 64);
            else if (fam.kind == 1) text = generate_dna_sequence(n);
            else if (fam.kind == 2) text = generate_source_code(n);
            else text = generate_periodic_string(n);

            // Simular DC3
            CacheHierarchySimulator sim_dc3;
            {
                std::vector<int> s(n + 3, 0);
                for (int i = 0; i < n; ++i) s[i] = static_cast<int>(static_cast<unsigned char>(text[i])) + 1;
                std::vector<int> SA(n, 0);
                sim_dc3_suffixArray(s.data(), SA.data(), n, 256, sim_dc3);
            }

            // Simular SA-IS
            CacheHierarchySimulator sim_sais;
            {
                std::vector<int> s(n + 1, 0);
                for (int i = 0; i < n; ++i) s[i] = static_cast<int>(static_cast<unsigned char>(text[i])) + 1;
                std::vector<int> SA(n + 1, -1);
                sim_sais_core(s.data(), SA.data(), n, 256, sim_sais);
            }

            const double dc3_ind_pct = 100.0 * static_cast<double>(sim_dc3.indirect_accesses) / sim_dc3.total_accesses;
            const double dc3_l1_rate = 100.0 * static_cast<double>(sim_dc3.l1_misses) / sim_dc3.total_accesses;
            const double dc3_l1_mpc  = static_cast<double>(sim_dc3.l1_misses) / n;

            const double sais_ind_pct = 100.0 * static_cast<double>(sim_sais.indirect_accesses) / sim_sais.total_accesses;
            const double sais_l1_rate = 100.0 * static_cast<double>(sim_sais.l1_misses) / sim_sais.total_accesses;
            const double sais_l1_mpc  = static_cast<double>(sim_sais.l1_misses) / n;

            const double l1_ratio = static_cast<double>(sim_dc3.l1_misses) / std::max<std::uint64_t>(1, sim_sais.l1_misses);
            const double l2_ratio = static_cast<double>(sim_dc3.l2_misses) / std::max<std::uint64_t>(1, sim_sais.l2_misses);

            std::cout << std::left << std::setw(13) << fam.name << std::right << " | "
                      << std::setw(7) << n << " | "
                      << std::setw(10) << std::fixed << std::setprecision(2) << dc3_l1_mpc
                      << " (" << std::setw(5) << dc3_l1_rate << "%)   | "
                      << std::setw(11) << std::fixed << std::setprecision(2) << sais_l1_mpc
                      << " (" << std::setw(5) << sais_l1_rate << "%)   | "
                      << std::setw(15) << std::fixed << std::setprecision(2) << l1_ratio << "x | "
                      << std::setw(15) << std::fixed << std::setprecision(2) << l2_ratio << "x\n";

            csv << fam.name << "," << n << ","
                << sim_dc3.total_accesses << "," << std::fixed << std::setprecision(4) << dc3_ind_pct << ","
                << sim_dc3.l1_misses << "," << dc3_l1_rate << "," << sim_dc3.l2_misses << "," << dc3_l1_mpc << ","
                << sim_sais.total_accesses << "," << sais_ind_pct << ","
                << sim_sais.l1_misses << "," << sais_l1_rate << "," << sim_sais.l2_misses << "," << sais_l1_mpc << ","
                << l1_ratio << "," << l2_ratio << "\n";
        }
    }
    std::cout << "-----------------------------------------------------------------------------------------------------------------\n";
    std::cout << "[OK] Metricas de cache exportadas a: " << csv_path << "\n";
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    const int repetitions = 5;
    const int max_naive_n = 10000;

    // Modo 1: Perfilado de cache (--profile-cache <output_csv>)
    if (argc >= 3 && std::string(argv[1]) == "--profile-cache") {
        return run_cache_profiler_mode(argv[2]);
    }

    // Modo 2: Evaluacion exclusiva de archivos de corpus (--only-files <output_csv> <file1> ...)
    if (argc >= 4 && std::string(argv[1]) == "--only-files") {
        const std::string out_csv = argv[2];
        std::vector<BenchmarkRow> file_results;
        std::cout << "========================================================================================================\n";
        std::cout << " BENCHMARK SOBRE ARCHIVOS DE CORPUS EN data/\n";
        std::cout << "========================================================================================================\n";
        std::cout << "Archivo                      |        n |  DC3(ms) | SAIS(ms) | Speedup | Depth(D/S) | Ratio(D / S) | B/char(D/S/Comp)\n";
        std::cout << "--------------------------------------------------------------------------------------------------------\n";
        for (int argi = 3; argi < argc; ++argi) {
            const std::string filepath = argv[argi];
            std::ifstream fin(filepath, std::ios::binary);
            if (!fin) continue;
            std::ostringstream buffer;
            buffer << fin.rdbuf();
            const std::string content = buffer.str();
            if (content.empty()) continue;

            const BenchmarkRow r = run_single_benchmark(filepath, content, repetitions, max_naive_n);
            file_results.push_back(r);
            std::cout << std::left << std::setw(28) << r.dataset << std::right << " | "
                      << std::setw(8) << r.n << " | "
                      << std::setw(8) << std::fixed << std::setprecision(2) << r.dc3_median_ms << " | "
                      << std::setw(8) << std::fixed << std::setprecision(2) << r.sais_median_ms << " | "
                      << std::setw(6) << std::fixed << std::setprecision(2) << r.speedup_dc3_over_sais << "x | "
                      << std::setw(4) << r.dc3_max_depth << " / " << std::setw(3) << r.sais_max_depth << " | "
                      << std::setw(5) << std::fixed << std::setprecision(3) << r.dc3_reduction_ratio << " / "
                      << std::setw(5) << std::fixed << std::setprecision(3) << r.sais_reduction_ratio << " | "
                      << std::setw(5) << std::fixed << std::setprecision(2) << r.dc3_bytes_per_char << "/"
                      << std::setw(5) << r.sais_bytes_per_char << "/"
                      << std::setw(4) << r.sais_compact_bpc << "\n";
        }
        std::cout << "--------------------------------------------------------------------------------------------------------\n";
        write_benchmark_csv(out_csv, file_results);
        std::cout << "[OK] Resultados de corpus exportados a: " << out_csv << "\n";
        return 0;
    }

    // Modo por defecto: Suite completa de escalamiento por familias + archivos opcionales
    std::cout << "========================================================================================================\n";
    std::cout << " ETAPA 4: BENCHMARK EXPERIMENTAL DC3 (Karkkainen & Sanders) vs SA-IS (Nong, Zhang & Chan)\n";
    std::cout << "========================================================================================================\n\n";

    const std::vector<int> sizes = {2000, 10000, 50000, 100000, 500000, 1000000, 2000000};
    std::vector<BenchmarkRow> results;

    struct FamilySpec {
        std::string name;
        int kind;
    };
    const std::vector<FamilySpec> families = {
        {"random_ascii", 0},
        {"dna_genome",   1},
        {"source_code",  2},
        {"periodic_str", 3}
    };

    for (const auto& fam : families) {
        std::cout << ">>> Distribucion: " << fam.name << " (" << repetitions << " repeticiones por punto)\n";
        std::cout << "--------------------------------------------------------------------------------------------------------\n";
        std::cout << "       n | Naive(ms) |  DC3(ms) | SAIS(ms) | Speedup | Depth(D/S) | Ratio(D / S) |  B/char (DC3 / SAIS / Comp)\n";
        std::cout << "--------------------------------------------------------------------------------------------------------\n";

        for (int n : sizes) {
            std::string text;
            if (fam.kind == 0) text = generate_random_text(n, 64);
            else if (fam.kind == 1) text = generate_dna_sequence(n);
            else if (fam.kind == 2) text = generate_source_code(n);
            else text = generate_periodic_string(n);

            const BenchmarkRow r = run_single_benchmark(fam.name, text, repetitions, max_naive_n);
            results.push_back(r);

            std::cout << std::setw(8) << r.n << " | ";
            if (r.naive_ms >= 0.0) {
                std::cout << std::setw(9) << std::fixed << std::setprecision(2) << r.naive_ms << " | ";
            } else {
                std::cout << "      --- | ";
            }
            std::cout << std::setw(8) << std::fixed << std::setprecision(2) << r.dc3_median_ms << " | "
                      << std::setw(8) << std::fixed << std::setprecision(2) << r.sais_median_ms << " | "
                      << std::setw(6) << std::fixed << std::setprecision(2) << r.speedup_dc3_over_sais << "x | "
                      << std::setw(4) << r.dc3_max_depth << " / " << std::setw(3) << r.sais_max_depth << " | "
                      << std::setw(5) << std::fixed << std::setprecision(3) << r.dc3_reduction_ratio << " / "
                      << std::setw(5) << std::fixed << std::setprecision(3) << r.sais_reduction_ratio << " | "
                      << std::setw(6) << std::fixed << std::setprecision(2) << r.dc3_bytes_per_char << " / "
                      << std::setw(6) << std::fixed << std::setprecision(2) << r.sais_bytes_per_char << " / "
                      << std::setw(5) << std::fixed << std::setprecision(2) << r.sais_compact_bpc << "\n";
        }
        std::cout << "--------------------------------------------------------------------------------------------------------\n\n";
    }

    const std::string csv_path = "results/benchmark_metrics.csv";
    write_benchmark_csv(csv_path, results);
    std::cout << "[OK] Resultados exportados exitosamente a: " << csv_path << "\n";

    const std::size_t os_rss_kb = get_os_peak_rss_bytes() / 1024;
    if (os_rss_kb > 0) {
        std::cout << "[INFO] Pico de memoria RSS del proceso reportado por el SO: " << os_rss_kb << " KB ("
                  << std::fixed << std::setprecision(2) << (static_cast<double>(os_rss_kb) / 1024.0) << " MB)\n";
    }

    return 0;
}
