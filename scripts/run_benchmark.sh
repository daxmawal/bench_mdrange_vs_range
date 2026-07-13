#!/usr/bin/env bash

set -euo pipefail

if (( $# < 2 )); then
  echo "Usage: $0 <build-directory> <output.json> [benchmark options...]" >&2
  exit 1
fi

build_directory=${1%/}
output_file=$2
shift 2

executable="${build_directory}/bench_range_vs_mdrange_triad_1d"

if [[ ! -x ${executable} ]]; then
  echo "Benchmark executable not found: ${executable}" >&2
  exit 1
fi

if [[ ${output_file} != *.json ]]; then
  echo "The output file must use the .json extension" >&2
  exit 1
fi

if [[ -e ${output_file} ]]; then
  echo "Refusing to overwrite existing result: ${output_file}" >&2
  exit 1
fi

mkdir -p "$(dirname "${output_file}")"
unset KOKKOS_TOOLS_LIBS KOKKOS_TOOLS_ARGS

"${executable}" \
  --benchmark_out="${output_file}" \
  --benchmark_out_format=json \
  "$@"

echo "Results written to ${output_file}"
