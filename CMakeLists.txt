cmake_minimum_required(VERSION 3.22)

project(range_vs_mdrange_triad_1d LANGUAGES CXX)

set(TRIAD_KOKKOS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/kokkos")
if(NOT EXISTS "${TRIAD_KOKKOS_SOURCE_DIR}/CMakeLists.txt")
  message(
    FATAL_ERROR
      "Kokkos submodule not found. Run: git submodule update --init --recursive"
  )
endif()

# Keep Kokkos' own auxiliary targets out of this benchmark build by default.
# Explicit command-line values (for example -DKokkos_ENABLE_TESTS=ON) win.
foreach(option_name IN ITEMS
        Kokkos_ENABLE_BENCHMARKS
        Kokkos_ENABLE_EXAMPLES
        Kokkos_ENABLE_TESTS)
  if(NOT DEFINED ${option_name})
    set(${option_name} OFF CACHE BOOL "" FORCE)
  endif()
endforeach()

add_subdirectory("${TRIAD_KOKKOS_SOURCE_DIR}" external/kokkos EXCLUDE_FROM_ALL)

set(TRIAD_BENCHMARK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/benchmark")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

if(EXISTS "${TRIAD_BENCHMARK_SOURCE_DIR}/CMakeLists.txt")
  add_subdirectory("${TRIAD_BENCHMARK_SOURCE_DIR}" external/benchmark
                   EXCLUDE_FROM_ALL)
else()
  find_package(benchmark 1.8.3 CONFIG QUIET)
  if(NOT benchmark_FOUND)
    message(
      FATAL_ERROR
        "Google Benchmark submodule not found. Run: git submodule update --init --recursive"
    )
  endif()
endif()

if(NOT DEFINED TRIAD_KOKKOS_GIT_SHA)
  set(TRIAD_KOKKOS_GIT_SHA "unknown")
  find_package(Git QUIET)
  if(Git_FOUND)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${TRIAD_KOKKOS_SOURCE_DIR}"
              rev-parse --short=12 HEAD
      RESULT_VARIABLE git_result
      OUTPUT_VARIABLE detected_kokkos_git_sha
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(git_result EQUAL 0 AND NOT detected_kokkos_git_sha STREQUAL "")
      set(TRIAD_KOKKOS_GIT_SHA "${detected_kokkos_git_sha}")

      execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${TRIAD_KOKKOS_SOURCE_DIR}"
                status --porcelain --untracked-files=no
        RESULT_VARIABLE git_status_result
        OUTPUT_VARIABLE kokkos_git_status
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if(git_status_result EQUAL 0 AND NOT kokkos_git_status STREQUAL "")
        string(APPEND TRIAD_KOKKOS_GIT_SHA "-dirty")
      endif()
    endif()
  endif()
endif()

add_executable(bench_range_vs_mdrange_triad_1d src/triad_1d.cpp)
target_compile_features(bench_range_vs_mdrange_triad_1d PRIVATE cxx_std_20)
target_compile_definitions(
  bench_range_vs_mdrange_triad_1d
  PRIVATE TRIAD_KOKKOS_GIT_SHA="${TRIAD_KOKKOS_GIT_SHA}"
)
target_link_libraries(
  bench_range_vs_mdrange_triad_1d
  PRIVATE Kokkos::kokkos benchmark::benchmark
)
