#include "sais.hpp"

#include <algorithm>
#include <chrono>
#include <vector>

namespace eda {

namespace {

struct SAISTracker {
    SAISStats* stats = nullptr;
    std::size_t current_aux_bytes = 0;
    std::size_t current_compact_aux_bytes = 0;
    std::size_t peak_compact_aux_bytes = 0;

    void add_level_memory(int n, int K) {
        const std::size_t bytes = (static_cast<std::size_t>(n + 1) + static_cast<std::size_t>(K + 1)) * sizeof(int);
        current_aux_bytes += bytes;
        // En el modelo compacto de Nong et al. (2009), el arreglo de tipos t usa 1 bit por caracter ((n+8)/8 bytes)
        // y bkt se libera antes de entrar a la llamada recursiva
        const std::size_t t_bits_bytes = static_cast<std::size_t>(n + 8) / 8;
        const std::size_t bkt_bytes = static_cast<std::size_t>(K + 1) * sizeof(int);
        if (stats != nullptr) {
            stats->peak_aux_bytes = std::max(stats->peak_aux_bytes, current_aux_bytes);
            peak_compact_aux_bytes = std::max(peak_compact_aux_bytes, current_compact_aux_bytes + t_bits_bytes + bkt_bytes);
        }
        current_compact_aux_bytes += t_bits_bytes;
    }

    void remove_level_memory(int n, int K) {
        const std::size_t bytes = (static_cast<std::size_t>(n + 1) + static_cast<std::size_t>(K + 1)) * sizeof(int);
        current_aux_bytes -= bytes;
        const std::size_t t_bits_bytes = static_cast<std::size_t>(n + 8) / 8;
        current_compact_aux_bytes -= t_bits_bytes;
    }
};

void saisCoreInternal(const int* s, int* SA, int n, int K, int depth, SAISTracker* tracker) {
    if (tracker != nullptr && tracker->stats != nullptr) {
        tracker->stats->max_recursion_depth = std::max(tracker->stats->max_recursion_depth, depth);
        tracker->add_level_memory(n, K);
    }

    // 1. Clasificar cada posicion i in [0..n] en L_TYPE (0) o S_TYPE (1)
    std::vector<int> t(n + 1, 0);
    classifyLS(s, t.data(), n);

    // 2. Etapa 1: Reducir el problema en al menos 1/2 ordenando todas las subcadenas LMS
    std::vector<int> bkt(K + 1, 0);
    getBuckets(s, bkt.data(), n, K, true);
    for (int i = 0; i <= n; ++i) {
        SA[i] = -1;
    }
    for (int i = 1; i <= n; ++i) {
        if (isLMS(i, t.data())) {
            SA[bkt[s[i]]--] = i;
        }
    }
    induceSAl(t.data(), SA, s, bkt.data(), n, K);
    induceSAs(t.data(), SA, s, bkt.data(), n, K);

    // 3. Compactar todas las subcadenas LMS ya ordenadas en las primeras n1 casillas de SA[0..n1-1]
    int n1 = 0;
    for (int i = 0; i <= n; ++i) {
        if (isLMS(SA[i], t.data())) {
            SA[n1++] = SA[i];
        }
    }

    if (tracker != nullptr && tracker->stats != nullptr && depth == 0 && n > 0) {
        tracker->stats->first_level_reduction_ratio = static_cast<double>(n1) / static_cast<double>(n);
    }

    // 4. Asignar nombres lexicograficos a las subcadenas LMS para formar s1 en SA[n1..n]
    for (int i = n1; i <= n; ++i) {
        SA[i] = -1;
    }
    int name = 0;
    int prev = -1;
    for (int i = 0; i < n1; ++i) {
        int pos = SA[i];
        if (!lmsSubstringsAreEqual(s, t.data(), n, prev, pos)) {
            name++;
            prev = pos;
        }
        SA[n1 + pos / 2] = name - 1;
    }

    for (int i = n, j = n; i >= n1; --i) {
        if (SA[i] >= 0) {
            SA[j--] = SA[i];
        }
    }

    // 5. Etapa 2: Resolver el problema reducido (SA1 a partir de s1)
    int* SA1 = SA;
    int* s1 = SA + n + 1 - n1;
    if (name < n1) {
        saisCoreInternal(s1, SA1, n1 - 1, name - 1, depth + 1, tracker);
    } else {
        for (int i = 0; i < n1; ++i) {
            SA1[s1[i]] = i;
        }
    }

    // 6. Etapa 3: Inducir el arreglo de sufijos completo SA[0..n] a partir de SA1
    getBuckets(s, bkt.data(), n, K, true);
    for (int i = 1, j = 0; i <= n; ++i) {
        if (isLMS(i, t.data())) {
            s1[j++] = i;
        }
    }
    for (int i = 0; i < n1; ++i) {
        SA1[i] = s1[SA1[i]];
    }
    for (int i = n1; i <= n; ++i) {
        SA[i] = -1;
    }
    for (int i = n1 - 1; i >= 0; --i) {
        int j = SA[i];
        SA[i] = -1;
        SA[bkt[s[j]]--] = j;
    }
    induceSAl(t.data(), SA, s, bkt.data(), n, K);
    induceSAs(t.data(), SA, s, bkt.data(), n, K);

    if (tracker != nullptr) {
        tracker->remove_level_memory(n, K);
    }
}

} // namespace

void classifyLS(const int* s, int* t, int n) {
    t[n] = S_TYPE;
    for (int i = n - 1; i >= 0; --i) {
        if (s[i] < s[i + 1]) {
            t[i] = S_TYPE;
        } else if (s[i] > s[i + 1]) {
            t[i] = L_TYPE;
        } else {
            t[i] = t[i + 1];
        }
    }
}

void getBuckets(const int* s, int* bkt, int n, int K, bool end) {
    for (int i = 0; i <= K; ++i) {
        bkt[i] = 0;
    }
    for (int i = 0; i <= n; ++i) {
        bkt[s[i]]++;
    }
    int sum = 0;
    for (int i = 0; i <= K; ++i) {
        sum += bkt[i];
        bkt[i] = end ? (sum - 1) : (sum - bkt[i]);
    }
}

void induceSAl(const int* t, int* SA, const int* s, int* bkt, int n, int K) {
    getBuckets(s, bkt, n, K, false);
    for (int i = 0; i <= n; ++i) {
        int j = SA[i] - 1;
        if (j >= 0 && t[j] == L_TYPE) {
            SA[bkt[s[j]]++] = j;
        }
    }
}

void induceSAs(const int* t, int* SA, const int* s, int* bkt, int n, int K) {
    getBuckets(s, bkt, n, K, true);
    for (int i = n; i >= 0; --i) {
        int j = SA[i] - 1;
        if (j >= 0 && t[j] == S_TYPE) {
            SA[bkt[s[j]]--] = j;
        }
    }
}

void sais_core(const int* s, int* SA, int n, int K) {
    saisCoreInternal(s, SA, n, K, 0, nullptr);
}

std::vector<int> build_sais_sa(const std::string& str, SAISStats* stats) {
    const auto t_start = std::chrono::steady_clock::now();
    if (stats != nullptr) {
        *stats = SAISStats{};
    }

    const int n = static_cast<int>(str.size());
    if (n == 0) {
        return {};
    }
    if (n == 1) {
        if (stats != nullptr) {
            const auto t_end = std::chrono::steady_clock::now();
            stats->elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
            stats->total_peak_bytes = sizeof(int);
            stats->bytes_per_char = static_cast<double>(sizeof(int));
            stats->compact_bytes_per_char = 5.0;
        }
        return {0};
    }

    std::vector<int> s(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        s[i] = static_cast<int>(static_cast<unsigned char>(str[i])) + 1;
    }
    s[n] = 0;

    std::vector<int> SA(n + 1, -1);
    SAISTracker tracker{stats, 0, 0, 0};
    saisCoreInternal(s.data(), SA.data(), n, 256, 0, stats != nullptr ? &tracker : nullptr);

    if (stats != nullptr) {
        const auto t_end = std::chrono::steady_clock::now();
        stats->elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        const std::size_t base_bytes = 2 * static_cast<std::size_t>(n + 1) * sizeof(int);
        stats->total_peak_bytes = base_bytes + stats->peak_aux_bytes;
        stats->bytes_per_char = static_cast<double>(stats->total_peak_bytes) / static_cast<double>(n);

        // En el modelo compacto de Nong et al. (Tabla 1): s ocupa 1B/char, SA ocupa 4B/char y t ocupa 1 bit/char
        const std::size_t compact_base = static_cast<std::size_t>(n + 1) * (1 + sizeof(int));
        stats->compact_bytes_per_char =
            static_cast<double>(compact_base + tracker.peak_compact_aux_bytes) / static_cast<double>(n);
    }

    return std::vector<int>(SA.begin() + 1, SA.end());
}

} // namespace eda
