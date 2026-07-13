# RangePolicy vs MDRangePolicy

## Clone

```bash
git clone --recurse-submodules git@github.com:daxmawal/bench_mdrange_vs_range.git
cd bench_mdrange_vs_range
```

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Run

```bash
./scripts/run_benchmark.sh \
  build \
  results/H100_range_vs_mdrange_1d.json
```

```bash
./scripts/run_benchmark.sh \
  build \
  results/H100_range_vs_mdrange_1d_repeated.json \
  --benchmark_min_warmup_time=2 \
  --benchmark_min_time=1s \
  --benchmark_repetitions=10 \
  --benchmark_enable_random_interleaving=true \
  --benchmark_display_aggregates_only=true
```

### Jean Zay — NVIDIA H100

```bash
srun --pty -A jza@h100 -C h100 --partition=gpu_p6 \
  --nodes=1 --ntasks-per-node=1 --cpus-per-task=24 \
  --gres=gpu:1 --hint=nomultithread bash

module purge
module load arch/h100
module load gcc/13.3.0
module load cuda/12.8.0
module load cmake


cmake -S . -B build-jean-zay-h100 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER="$PWD/external/kokkos/bin/nvcc_wrapper" \
  -DKokkos_ENABLE_CUDA=ON \
  -DKokkos_ARCH_HOPPER90=ON
cmake --build build-jean-zay-h100 --parallel 8

./scripts/run_benchmark.sh \
  build-jean-zay-h100 \
  results/jean_zay_H100/H100_range_vs_mdrange_1d.json
```

### Adastra — AMD MI300A

```bash
module purge
module load cmake
module load rocm
module load craype-accel-amd-gfx90a

cmake -S . -B build-adastra-mi300a \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER="$(command -v hipcc)" \
  -DKokkos_ENABLE_HIP=ON \
  -DKokkos_ARCH_AMD_GFX942_APU=ON
cmake --build build-adastra-mi300a --parallel 8

sbatch scripts/supercomputers/adastra_mi300a.slurm \
  build-adastra-mi300a \
  results/adastra_MI300A/MI300A_range_vs_mdrange_1d.json
```

### Ruche — NVIDIA A100

```bash
module purge
module restore v100_compil

cmake -S . -B build-ruche-a100 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER="$PWD/external/kokkos/bin/nvcc_wrapper" \
  -DKokkos_ENABLE_CUDA=ON \
  -DKokkos_ARCH_AMPERE80=ON
cmake --build build-ruche-a100 --parallel 8

sbatch scripts/supercomputers/ruche_a100.slurm \
  build-ruche-a100 \
  results/ruche_A100/A100_range_vs_mdrange_1d.json
```
