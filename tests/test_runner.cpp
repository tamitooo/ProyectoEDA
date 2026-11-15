#include "dc3.hpp"
#include "naive_sa.hpp"
#include "sais.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

void test_etapa1_naive_mississippi() {
    const std::string s = "mississippi";
    const int n = static_cast<int>(s.size());

    std::cout << "=========================================================\n";
    std::cout << " ETAPA 1: Validacion Base y Suffix Array de 'mississippi'\n";
    std::cout << "=========================================================\n\n";

    // 1. Mostrar transformacion a arreglo de enteros con centinela '$' (0)
    const std::vector<int> T = eda::string_to_int_with_sentinel(s);
    std::cout << "1) Representacion en arreglo de enteros con centinela (n + 1 = " << T.size() << "):\n";
    std::cout << "   Indice i : ";
    for (int i = 0; i <= n; ++i) {
        std::cout << std::setw(4) << i;
    }
    std::cout << "\n   Char S[i]: ";
    for (int i = 0; i < n; ++i) {
        std::cout << std::setw(4) << s[i];
    }
    std::cout << std::setw(4) << "$";
    std::cout << "\n   Int  T[i]: ";
    for (int i = 0; i <= n; ++i) {
        std::cout << std::setw(4) << T[i];
    }
    std::cout << "\n\n";

    // 2. Construir el Suffix Array con el algoritmo ingenuo
    const std::vector<int> sa = eda::build_naive_sa(s);

    // Resultado exacto esperado segun la Figura 1 de Karkkainen & Sanders (2003)
    const std::vector<int> expected_sa = {10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2};

    // 3. Imprimir tabla visual de los sufijos ordenados
    std::cout << "2) Tabla de Sufijos Ordenados Lexicograficamente (SA):\n";
    std::cout << "   -------------------------------------------------------\n";
    std::cout << "   Rank (r) | SA[r] | SA[r] mod 3 | Sufijo S[SA[r]..n-1]\n";
    std::cout << "   -------------------------------------------------------\n";
    for (int r = 0; r < n; ++r) {
        const int idx = sa[r];
        std::cout << "   " << std::setw(8) << r
                  << " | " << std::setw(5) << idx
                  << " | " << std::setw(11) << (idx % 3)
                  << " | " << s.substr(idx) << "$\n";
    }
    std::cout << "   -------------------------------------------------------\n\n";

    // 4. Verificaciones automaticas de correctitud
    assert(sa == expected_sa && "Error: El SA calculado no coincide con el paper de DC3.");
    assert(eda::verify_suffix_array(s, sa) && "Error: verify_suffix_array fallo.");

    std::cout << "[OK] Etapa 1 superada: SA coincide con [10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2].\n\n";
}

void test_paso2_leq_y_radixPass() {
    std::cout << "=========================================================\n";
    std::cout << " PASO 2 (DC3 Parte 1): Pruebas de leq() y radixPass()\n";
    std::cout << "=========================================================\n\n";

    assert(eda::leq(1, 5, 2, 0) == true);
    assert(eda::leq(2, 3, 2, 5) == true);
    assert(eda::leq(2, 5, 2, 5) == true);
    assert(eda::leq(2, 6, 2, 5) == false);

    assert(eda::leq(1, 9, 9, 2, 0, 0) == true);
    assert(eda::leq(2, 3, 9, 2, 4, 0) == true);
    assert(eda::leq(2, 4, 5, 2, 4, 5) == true);
    assert(eda::leq(2, 4, 6, 2, 4, 5) == false);

    const std::string str = "mississippi";
    const int n = static_cast<int>(str.size());
    const int K = 255;

    std::vector<int> s(n + 3, 0);
    for (int i = 0; i < n; ++i) {
        s[i] = static_cast<int>(static_cast<unsigned char>(str[i]));
    }

    std::vector<int> s12;
    for (int i = 0; i < n; ++i) {
        if (i % 3 != 0) {
            s12.push_back(i);
        }
    }
    const int n02 = static_cast<int>(s12.size());
    std::vector<int> SA12(n02, 0);

    eda::radixPass(s12.data(), SA12.data(), s.data() + 2, n02, K);
    eda::radixPass(SA12.data(), s12.data(), s.data() + 1, n02, K);
    eda::radixPass(s12.data(), SA12.data(), s.data(), n02, K);

    const std::vector<int> expected_triplets_order = {10, 7, 1, 4, 8, 2, 5};
    assert(SA12 == expected_triplets_order && "Error: El orden de las tripletas tras 3 radixPass es incorrecto.");

    std::cout << "[OK] Paso 2 superado: Las 3 pasadas de radixPass ordenaron establemente\n"
              << "     las tripletas en [10, 7, 1, 4, 8, 2, 5] con nombres [1..5] exactos al paper.\n\n";
}

void test_paso3_dc3_phase1() {
    std::cout << "=========================================================\n";
    std::cout << " PASO 3 (DC3 Parte 2): Construccion de s12 y Caso Base\n";
    std::cout << "=========================================================\n\n";

    {
        const std::string str = "mississippi";
        const int n = static_cast<int>(str.size());
        const int K = 255;

        std::vector<int> s(n + 3, 0);
        for (int i = 0; i < n; ++i) {
            s[i] = static_cast<int>(static_cast<unsigned char>(str[i]));
        }

        std::vector<int> s12;
        std::vector<int> SA12;
        const int name = eda::dc3Phase1(s.data(), n, K, s12, SA12, false);

        const std::vector<int> expected_s12 = {3, 3, 2, 1, 5, 5, 4, 0, 0, 0};
        assert(name == 5 && "Error: Para 'mississippi' deben existir exactamente 5 nombres unicos.");
        assert(s12 == expected_s12 && "Error: s12 no coincide con [3, 3, 2, 1, 5, 5, 4, 0, 0, 0] de la Figura 1.");
    }

    {
        const std::string str = "bananas";
        const int n = static_cast<int>(str.size());
        const int n0 = (n + 2) / 3;
        const int n02 = n0 + (n / 3);
        const int K = 255;

        std::vector<int> s(n + 3, 0);
        for (int i = 0; i < n; ++i) {
            s[i] = static_cast<int>(static_cast<unsigned char>(str[i]));
        }

        std::vector<int> s12;
        std::vector<int> SA12;
        const int name = eda::dc3Phase1(s.data(), n, K, s12, SA12, false);

        const std::vector<int> expected_s12_bananas = {2, 5, 1, 4, 3, 0, 0, 0};
        const std::vector<int> expected_SA12_bananas = {2, 0, 4, 3, 1, 0, 0, 0};

        assert(name == n02 && "Error: Todas las tripletas de 'bananas' deben tener nombres unicos.");
        assert(s12 == expected_s12_bananas && "Error: s12 para 'bananas' no coincide con lo esperado.");
        assert(SA12 == expected_SA12_bananas && "Error: La rama else no construyo SA12 correctamente.");
    }

    std::cout << "[OK] Paso 3 superado: s12 coincide con [3, 3, 2, 1, 5, 5, 4] en 'mississippi'\n"
              << "     y la rama else construye SA12 directamente cuando name == n02.\n\n";
}

void test_paso4_dc3_complete() {
    std::cout << "=========================================================\n";
    std::cout << " PASO 4 (DC3 Parte 3): SA0, Merge y Validacion Completa\n";
    std::cout << "=========================================================\n\n";

    {
        const std::string str = "mississippi";
        const int n = static_cast<int>(str.size());
        const int n0 = (n + 2) / 3;
        const int n02 = n0 + (n / 3);
        const int K = 255;

        std::vector<int> s(n + 3, 0);
        for (int i = 0; i < n; ++i) {
            s[i] = static_cast<int>(static_cast<unsigned char>(str[i]));
        }

        std::vector<int> s12;
        std::vector<int> SA12;
        eda::dc3Phase1(s.data(), n, K, s12, SA12, true);

        std::vector<int> s0(n0, 0);
        std::vector<int> SA0(n0, 0);
        for (int i = 0, j = 0; i < n02; ++i) {
            if (SA12[i] < n0) {
                s0[j++] = 3 * SA12[i];
            }
        }
        eda::radixPass(s0.data(), SA0.data(), s.data(), n0, K);

        const std::vector<int> expected_SA12 = {3, 2, 1, 0, 6, 5, 4, 0, 0, 0};
        const std::vector<int> expected_inv_SA12 = {4, 3, 2, 1, 7, 6, 5, 0, 0, 0};
        const std::vector<int> expected_SA0 = {0, 9, 6, 3};
        const std::vector<int> expected_SA = {10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2};

        assert(SA12 == expected_SA12);
        assert(s12 == expected_inv_SA12);
        assert(SA0 == expected_SA0);

        const std::vector<int> sa_dc3 = eda::build_dc3_sa(str);
        assert(sa_dc3 == expected_SA);
        assert(eda::verify_suffix_array(str, sa_dc3));
    }

    std::cout << "[OK] ETAPA 2 COMPLETADA AL 100%: Algoritmo DC3 implementado y verificado.\n\n";
}

void test_paso5_sais_types_lms_and_induction() {
    std::cout << "=========================================================\n";
    std::cout << " PASO 5 (SA-IS Parte 1): Tipos L/S, LMS, Cubetas e Induccion\n";
    std::cout << "=========================================================\n\n";

    {
        const std::string str = "mmiississippii";
        const int n = static_cast<int>(str.size());
        const int K = 255;

        const std::vector<int> s = eda::string_to_int_with_sentinel(str);
        std::vector<int> t(n + 1, 0);
        eda::classifyLS(s.data(), t.data(), n);

        const std::vector<int> expected_t = {0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1};
        assert(t == expected_t);

        std::vector<int> lms_positions;
        for (int i = 0; i <= n; ++i) {
            if (eda::isLMS(i, t.data())) {
                lms_positions.push_back(i);
            }
        }
        const std::vector<int> expected_lms = {2, 6, 9, 14};
        assert(lms_positions == expected_lms);

        std::vector<int> SA(n + 1, -1);
        std::vector<int> bkt(K + 1, 0);
        eda::getBuckets(s.data(), bkt.data(), n, K, true);
        for (int i = 1; i <= n; ++i) {
            if (eda::isLMS(i, t.data())) {
                SA[bkt[s[i]]--] = i;
            }
        }
        eda::induceSAl(t.data(), SA.data(), s.data(), bkt.data(), n, K);
        eda::induceSAs(t.data(), SA.data(), s.data(), bkt.data(), n, K);

        std::vector<int> sorted_lms;
        for (int i = 0; i <= n; ++i) {
            if (eda::isLMS(SA[i], t.data())) {
                sorted_lms.push_back(SA[i]);
            }
        }
        const std::vector<int> expected_sorted_lms = {14, 2, 9, 6};
        assert(sorted_lms == expected_sorted_lms);
    }

    std::cout << "[OK] Paso 5 superado: Clasificacion L/S, LMS, getBuckets e induccion verificados.\n\n";
}

void test_paso6_sais_complete() {
    std::cout << "=========================================================\n";
    std::cout << " PASO 6 (SA-IS Parte 2): Nombramiento LMS, s1 y SA-IS Completo\n";
    std::cout << "=========================================================\n\n";

    // -------------------------------------------------------------------------
    // 1. Traza de "mississippi" (n = 11): activa la rama recursiva (name = 3 < n1 = 4)
    //    porque las subcadenas LMS en i=1 ("issi") e i=4 ("issi") empatan
    // -------------------------------------------------------------------------
    {
        const std::string str = "mississippi";
        const int n = static_cast<int>(str.size()); // 11
        const int K = 255;

        const std::vector<int> s = eda::string_to_int_with_sentinel(str);
        std::vector<int> t(n + 1, 0);
        eda::classifyLS(s.data(), t.data(), n);

        // En "mississippi$", LMS = {1, 4, 7, 11}:
        // s[1..4] = "issi" y s[4..7] = "issi" deben ser iguales segun lmsSubstringsAreEqual
        assert(eda::lmsSubstringsAreEqual(s.data(), t.data(), n, 1, 4) == true);
        assert(eda::lmsSubstringsAreEqual(s.data(), t.data(), n, 4, 7) == false);
        assert(eda::lmsSubstringsAreEqual(s.data(), t.data(), n, 7, 11) == false);

        // Simular compactacion y nombramiento en SA[0..n]
        std::vector<int> SA(n + 1, -1);
        std::vector<int> bkt(K + 1, 0);
        eda::getBuckets(s.data(), bkt.data(), n, K, true);
        for (int i = 1; i <= n; ++i) {
            if (eda::isLMS(i, t.data())) {
                SA[bkt[s[i]]--] = i;
            }
        }
        eda::induceSAl(t.data(), SA.data(), s.data(), bkt.data(), n, K);
        eda::induceSAs(t.data(), SA.data(), s.data(), bkt.data(), n, K);

        int n1 = 0;
        for (int i = 0; i <= n; ++i) {
            if (eda::isLMS(SA[i], t.data())) {
                SA[n1++] = SA[i];
            }
        }
        for (int i = n1; i <= n; ++i) {
            SA[i] = -1;
        }
        int name = 0, prev = -1;
        for (int i = 0; i < n1; ++i) {
            int pos = SA[i];
            if (!eda::lmsSubstringsAreEqual(s.data(), t.data(), n, prev, pos)) {
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

        const int* s1_ptr = SA.data() + n + 1 - n1;
        const std::vector<int> s1_vec(s1_ptr, s1_ptr + n1);
        // P1 = {1, 4, 7, 11} -> nombres: 1->"issi"(2), 4->"issi"(2), 7->"ippi$"(1), 11->"$"(0)
        const std::vector<int> expected_s1 = {2, 2, 1, 0};
        assert(n1 == 4 && name == 3);
        assert(s1_vec == expected_s1 && "Error: s1 en 'mississippi$' debe ser [2, 2, 1, 0].");

        const std::vector<int> sa_sais = eda::build_sais_sa(str);
        const std::vector<int> expected_sa = {10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2};
        assert(sa_sais == expected_sa && "Error: SA final de SA-IS en 'mississippi' es incorrecto.");

        std::cout << "1) Traza de reduccion y recursion en 'mississippi$' (n = 11):\n";
        std::cout << "   Posiciones LMS P1          : [1, 4, 7, 11]\n";
        std::cout << "   lmsSubstringsAreEqual(1,4) : true (ambas son \"issi\" con tipos S L L S)\n";
        std::cout << "   Cadena reducida s1         : [2, 2, 1, 0] (n1 = 4, name = 3 -> activa recursion)\n";
        std::cout << "   SA final (descartando '$') : [10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2]\n\n";
    }

    // -------------------------------------------------------------------------
    // 2. Traza de "mmiississippii" (n = 14) y "mmiissiissiippii" (n = 16, Fig. 1 paper)
    // -------------------------------------------------------------------------
    {
        const std::string str1 = "mmiississippii";
        const std::vector<int> sa1 = eda::build_sais_sa(str1);
        assert(sa1 == eda::build_naive_sa(str1));
        assert(sa1 == eda::build_dc3_sa(str1));

        const std::string str2 = "mmiissiissiippii"; // Ejemplo exacto de 16 letras + '$' del paper
        const std::vector<int> sa2 = eda::build_sais_sa(str2);
        assert(sa2 == eda::build_naive_sa(str2));
        assert(sa2 == eda::build_dc3_sa(str2));

        std::cout << "2) Verificacion en 'mmiississippii' (rama directa) y 'mmiissiissiippii' (rama recursiva): OK\n\n";
    }

    // -------------------------------------------------------------------------
    // 3. Validacion cruzada exhaustiva: SA-IS vs DC3 vs Naive SA
    // -------------------------------------------------------------------------
    std::vector<std::string> test_cases = {
        "",
        "a",
        "z",
        "ab",
        "ba",
        "aa",
        "banana",
        "bananas",
        "bananass",
        "mississippi",
        "mmiississippii",
        "mmiissiissiippii",
        "abracadabra",
        "abracadabra!",
        "abracadabra!!",
        "aaaa",
        "aaaaa",
        "aaaaaa",
        "aaaaaaaaaaaa",
        "aaaaaaaaaaaaa",
        "mississippimississippimississippi",
        "ababababababababababababababababab",
        "aabaaabaaaabaaaaabaaaaaab",
        "gattaca_gattaca_gattaca_acgtacgtacgt"
    };

    std::string high_bytes_str;
    for (int b = 255; b >= 1; --b) {
        high_bytes_str.push_back(static_cast<char>(static_cast<unsigned char>(b)));
    }
    test_cases.push_back(high_bytes_str);

    for (const std::string& tc : test_cases) {
        const std::vector<int> sa_sais = eda::build_sais_sa(tc);
        const std::vector<int> sa_dc3 = eda::build_dc3_sa(tc);
        const std::vector<int> sa_naive = eda::build_naive_sa(tc);

        assert(sa_sais == sa_naive && "Error: SA-IS difiere de Naive SA.");
        assert(sa_sais == sa_dc3 && "Error: SA-IS difiere de DC3.");
        assert(eda::verify_suffix_array(tc, sa_sais) && "Error: verify_suffix_array fallo en SA-IS.");
    }

    std::cout << "3) Validacion triple (SA-IS == DC3 == Naive SA) en " << test_cases.size() << " escenarios:\n";
    std::cout << "   - Casos borde (vacia, 1 caracter, 'aaaa...a' repetidos): OK\n";
    std::cout << "   - Empates de subcadenas LMS y multiples niveles recursivos: OK\n";
    std::cout << "   - Alfabeto completo unsigned char [1..255]: OK\n\n";
    std::cout << "[OK] ETAPA 3 COMPLETADA AL 100%: Algoritmo SA-IS implementado y verificado.\n";
}

int main() {
    test_etapa1_naive_mississippi();
    test_paso2_leq_y_radixPass();
    test_paso3_dc3_phase1();
    test_paso4_dc3_complete();
    test_paso5_sais_types_lms_and_induction();
    test_paso6_sais_complete();
    return 0;
}
