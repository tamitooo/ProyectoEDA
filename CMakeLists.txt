cmake_minimum_required(VERSION 3.16)
project(ProyectoEDA LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Flags de advertencia y optimizacion para C++ nativo
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# Ejecutable de pruebas de correctitud (Etapas 1 a 3)
add_executable(test_runner
    tests/test_runner.cpp
    src/dc3.cpp
    src/sais.cpp
)

# Ejecutable de benchmarks experimentales (Etapa 4)
add_executable(benchmark
    bench/benchmark.cpp
    src/dc3.cpp
    src/sais.cpp
)
if(WIN32)
    target_link_libraries(benchmark PRIVATE psapi)
endif()
