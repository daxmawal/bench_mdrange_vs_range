// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <Kokkos_Core.hpp>
#include <benchmark/benchmark.h>

#include <cstdint>
#include <iostream>
#include <string>

namespace {

using ExecutionSpace = Kokkos::DefaultExecutionSpace;
using Index = std::uint32_t;
using IndexType = Kokkos::IndexType<Index>;
using View =
    Kokkos::View<double *, Kokkos::LayoutLeft, ExecutionSpace::memory_space>;
using RangePolicy = Kokkos::RangePolicy<ExecutionSpace, IndexType>;
using MDRangePolicy =
    Kokkos::MDRangePolicy<ExecutionSpace, Kokkos::Rank<1>, IndexType>;

constexpr std::int64_t size_2_pow_22 = 1LL << 22;
constexpr std::int64_t size_2_pow_24 = 1LL << 24;
constexpr std::int64_t size_2_pow_26 = 1LL << 26;
constexpr std::int64_t size_22_pow_6 = 113'379'904;
constexpr double scalar = 2.718281828;

struct Triad {
  View a;
  View b;
  View c;

  KOKKOS_INLINE_FUNCTION
  void operator()(Index i) const { c(i) = a(i) + scalar * b(i); }
};

bool result_is_valid(const View &c, Index size) {
  constexpr double expected = 1.0 + 2.0 * scalar;
  constexpr double tolerance = 1.0e-12;

  std::uint64_t errors = 0;
  Kokkos::parallel_reduce(
      "check_triad", RangePolicy(0, size),
      KOKKOS_LAMBDA(Index i, std::uint64_t & local_errors) {
        const double value = c(i);
        if (!(value >= expected - tolerance && value <= expected + tolerance)) {
          ++local_errors;
        }
      },
      errors);

  return errors == 0;
}

template <class Policy>
void run_triad(benchmark::State &state, const Policy &policy) {
  const auto size = static_cast<Index>(state.range(0));

  View a("a", size);
  View b("b", size);
  View c("c", size);

  Kokkos::deep_copy(a, 1.0);
  Kokkos::deep_copy(b, 2.0);
  Kokkos::deep_copy(c, 0.0);
  ExecutionSpace().fence();

  const Triad triad{a, b, c};

  for (auto _ : state) {
    Kokkos::Timer timer;
    Kokkos::parallel_for("triad", policy, triad);
    ExecutionSpace().fence();
    state.SetIterationTime(timer.seconds());
  }

  const std::int64_t bytes =
      3 * static_cast<std::int64_t>(size) * sizeof(double);
  const double megabytes = static_cast<double>(bytes) / 1'000'000.0;

  state.SetBytesProcessed(state.iterations() * bytes);
  state.counters["MB"] = benchmark::Counter(megabytes);
  state.counters["FOM: GB/s"] = benchmark::Counter(
      megabytes / 1'000.0, benchmark::Counter::kIsIterationInvariantRate);

  if (!result_is_valid(c, size)) {
    state.SkipWithError("Triad validation failed");
  }
}

template <int Rank> void RangePolicy_Triad(benchmark::State &state) {
  static_assert(Rank == 1);
  const auto size = static_cast<Index>(state.range(0));
  run_triad(state, RangePolicy(0, size));
}

template <int Rank> void MDRangePolicy_Triad(benchmark::State &state) {
  static_assert(Rank == 1);
  const auto size = static_cast<Index>(state.range(0));
  const MDRangePolicy::point_type begin{0};
  const MDRangePolicy::point_type end{size};
  run_triad(state, MDRangePolicy(begin, end));
}

void add_problem_sizes(benchmark::internal::Benchmark *benchmark) {
  benchmark->Arg(size_2_pow_22)
      ->Arg(size_2_pow_24)
      ->Arg(size_2_pow_26)
      ->Arg(size_22_pow_6)
      ->UseManualTime()
      ->Unit(benchmark::kMillisecond);
}

BENCHMARK_TEMPLATE(RangePolicy_Triad, 1)->Apply(add_problem_sizes);
BENCHMARK_TEMPLATE(MDRangePolicy_Triad, 1)->Apply(add_problem_sizes);

} // namespace

int main(int argc, char **argv) {
  Kokkos::ScopeGuard kokkos(argc, argv);
  benchmark::Initialize(&argc, argv);

  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }

  const std::string kokkos_version = std::to_string(KOKKOS_VERSION_MAJOR) +
                                     "." +
                                     std::to_string(KOKKOS_VERSION_MINOR) +
                                     "." + std::to_string(KOKKOS_VERSION_PATCH);
  benchmark::AddCustomContext("Kokkos version", kokkos_version);
  benchmark::AddCustomContext("Execution space", ExecutionSpace::name());

  Kokkos::print_configuration(std::cout, true);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
}
