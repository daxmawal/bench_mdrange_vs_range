#!/bin/bash

#SBATCH --job-name=range-vs-mdrange
#SBATCH --constraint=MI300
#SBATCH --nodes=1
#SBATCH --account=cad16293
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --gres=gpu:1
#SBATCH --cpus-per-task=8
#SBATCH --gpu-freq=high
#SBATCH --hint=nomultithread
#SBATCH --time=00:30:00
#SBATCH --exclusive
#SBATCH --output=bench_adastra_%j.out
#SBATCH --error=bench_adastra_%j.err

set -euo pipefail

if (( $# < 2 )); then
  echo "Usage: sbatch $0 <build-directory> <output.json> [benchmark options...]" >&2
  exit 1
fi

build_directory=$1
output_file=$2
shift 2

if ! module restore amd_rocm72; then
  echo "No module collection named 'amd_rocm72'; continuing with current modules." >&2
fi
module list

cd "${SLURM_SUBMIT_DIR}"
export HSA_XNACK=1

srun --ntasks=1 ./scripts/run_benchmark.sh \
  "${build_directory}" \
  "${output_file}" \
  --benchmark_repetitions=10 \
  "$@"
