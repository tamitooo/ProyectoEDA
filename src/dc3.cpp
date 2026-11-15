#include "dc3.hpp"

#include <algorithm>
#include <chrono>
#include <vector>

namespace eda {

namespace {

struct DC3Tracker {
    DC3Stats* stats = nullptr;
    std::size_t current_aux_bytes = 0;

    void add_bytes(std::size_t b) {
        current_aux_bytes += b;
        if (stats != nullptr) {
            stats->peak_aux_bytes = std::max(stats->peak_aux_bytes, current_aux_bytes);
        }
    }

    void remove_bytes(std::size_t b) {
        current_aux_bytes -= b;
    }

    void record_temp_radix_counter(int K) {
        if (stats != nullptr) {
            const std::size_t counter_bytes = static_cast<std::size_t>(K + 1) * sizeof(int);
            stats->peak_aux_bytes = std::max(stats->peak_aux_bytes, current_aux_bytes + counter_bytes);
        }
    }
};

void suffixArrayInternal(const int* s, int* SA, int n, int K, int depth, DC3Tracker* tracker);

int dc3Phase1Internal(
    const int* s,
    int n,
    int K,
    std::vector<int>& s12,
    std::vector<int>& SA12,
    bool recurse_on_ties,
    int depth,
    DC3Tracker* tracker
) {
    int n0 = (n + 2) / 3;
    int n1 = (n + 1) / 3;
    int n2 = n / 3;
    int n02 = n0 + n2;

    if (tracker != nullptr && tracker->stats != nullptr) {
        tracker->stats->max_recursion_depth = std::max(tracker->stats->max_recursion_depth, depth);
        if (depth == 0 && n > 0) {
            tracker->stats->first_level_reduction_ratio = static_cast<double>(n02) / static_cast<double>(n);
        }
    }

    s12.assign(n02 + 3, 0);
    s12[n02] = s12[n02 + 1] = s12[n02 + 2] = 0;

    SA12.assign(n02 + 3, 0);
    SA12[n02] = SA12[n02 + 1] = SA12[n02 + 2] = 0;

    const std::size_t s12_sa12_bytes = 2 * static_cast<std::size_t>(n02 + 3) * sizeof(int);
    if (tracker != nullptr) {
        tracker->add_bytes(s12_sa12_bytes);
    }

    for (int i = 0, j = 0; i < n + (n0 - n1); ++i) {
        if (i % 3 != 0) {
            s12[j++] = i;
        }
    }

    if (tracker != nullptr) {
        tracker->record_temp_radix_counter(K);
    }
    radixPass(s12.data(), SA12.data(), s + 2, n02, K);
    radixPass(SA12.data(), s12.data(), s + 1, n02, K);
    radixPass(s12.data(), SA12.data(), s,     n02, K);

    int name = 0, c0 = -1, c1 = -1, c2 = -1;
    for (int i = 0; i < n02; ++i) {
        if (s[SA12[i]] != c0 || s[SA12[i] + 1] != c1 || s[SA12[i] + 2] != c2) {
            name++;
            c0 = s[SA12[i]];
            c1 = s[SA12[i] + 1];
            c2 = s[SA12[i] + 2];
        }
        if (SA12[i] % 3 == 1) {
            s12[SA12[i] / 3] = name;
        } else {
            s12[SA12[i] / 3 + n0] = name;
        }
    }

    if (name < n02) {
        if (recurse_on_ties) {
            suffixArrayInternal(s12.data(), SA12.data(), n02, name, depth + 1, tracker);
            for (int i = 0; i < n02; ++i) {
                s12[SA12[i]] = i + 1;
            }
        }
    } else {
        for (int i = 0; i < n02; ++i) {
            SA12[s12[i] - 1] = i;
        }
    }

    return name;
}

void suffixArrayInternal(const int* s, int* SA, int n, int K, int depth, DC3Tracker* tracker) {
    if (n <= 0) {
        return;
    }
    if (n == 1) {
        SA[0] = 0;
        return;
    }

    int n0 = (n + 2) / 3;
    int n1 = (n + 1) / 3;
    int n2 = n / 3;
    int n02 = n0 + n2;

    std::vector<int> s12;
    std::vector<int> SA12;
    dc3Phase1Internal(s, n, K, s12, SA12, true, depth, tracker);

    std::vector<int> s0(n0, 0);
    std::vector<int> SA0(n0, 0);
    const std::size_t s0_sa0_bytes = 2 * static_cast<std::size_t>(n0) * sizeof(int);
    if (tracker != nullptr) {
        tracker->add_bytes(s0_sa0_bytes);
    }

    for (int i = 0, j = 0; i < n02; ++i) {
        if (SA12[i] < n0) {
            s0[j++] = 3 * SA12[i];
        }
    }
    if (tracker != nullptr) {
        tracker->record_temp_radix_counter(K);
    }
    radixPass(s0.data(), SA0.data(), s, n0, K);

    for (int p = 0, t = n0 - n1, k = 0; k < n; ++k) {
        auto GetI = [&]() -> int {
            return (SA12[t] < n0) ? (SA12[t] * 3 + 1) : ((SA12[t] - n0) * 3 + 2);
        };

        int i = GetI();
        int j = SA0[p];

        bool sa12_is_smaller = (SA12[t] < n0)
            ? leq(s[i], s12[SA12[t] + n0], s[j], s12[j / 3])
            : leq(s[i], s[i + 1], s12[SA12[t] - n0 + 1], s[j], s[j + 1], s12[j / 3 + n0]);

        if (sa12_is_smaller) {
            SA[k] = i;
            t++;
            if (t == n02) {
                for (k++; p < n0; p++, k++) {
                    SA[k] = SA0[p];
                }
            }
        } else {
            SA[k] = j;
            p++;
            if (p == n0) {
                for (k++; t < n02; t++, k++) {
                    SA[k] = GetI();
                }
            }
        }
    }

    if (tracker != nullptr) {
        const std::size_t s12_sa12_bytes = 2 * static_cast<std::size_t>(n02 + 3) * sizeof(int);
        tracker->remove_bytes(s12_sa12_bytes + s0_sa0_bytes);
    }
}

} // namespace

void radixPass(const int* a, int* b, const int* r, int n, int K) {
    std::vector<int> c(K + 1, 0);
    for (int i = 0; i < n; ++i) {
        c[r[a[i]]]++;
    }
    for (int i = 0, sum = 0; i <= K; ++i) {
        int t = c[i];
        c[i] = sum;
        sum += t;
    }
    for (int i = 0; i < n; ++i) {
        b[c[r[a[i]]]++] = a[i];
    }
}

int dc3Phase1(
    const int* s,
    int n,
    int K,
    std::vector<int>& s12,
    std::vector<int>& SA12,
    bool recurse_on_ties
) {
    return dc3Phase1Internal(s, n, K, s12, SA12, recurse_on_ties, 0, nullptr);
}

void suffixArray(const int* s, int* SA, int n, int K) {
    suffixArrayInternal(s, SA, n, K, 0, nullptr);
}

std::vector<int> build_dc3_sa(const std::string& str, DC3Stats* stats) {
    const auto t_start = std::chrono::steady_clock::now();
    if (stats != nullptr) {
        *stats = DC3Stats{};
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
        }
        return {0};
    }

    std::vector<int> s(n + 3, 0);
    for (int i = 0; i < n; ++i) {
        s[i] = static_cast<int>(static_cast<unsigned char>(str[i]));
    }

    std::vector<int> SA(n, 0);
    DC3Tracker tracker{stats, 0};
    suffixArrayInternal(s.data(), SA.data(), n, 255, 0, stats != nullptr ? &tracker : nullptr);

    if (stats != nullptr) {
        const auto t_end = std::chrono::steady_clock::now();
        stats->elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        const std::size_t base_bytes = (static_cast<std::size_t>(n + 3) + static_cast<std::size_t>(n)) * sizeof(int);
        stats->total_peak_bytes = base_bytes + stats->peak_aux_bytes;
        stats->bytes_per_char = static_cast<double>(stats->total_peak_bytes) / static_cast<double>(n);
    }

    return SA;
}

} // namespace eda
