#!/bin/bash
#SBATCH --job-name=pace_stride
#SBATCH --time=12:00:00
#SBATCH --mem=16G
#SBATCH --output=run_stride_%j.out
#SBATCH --error=run_stride_%j.err
#SBATCH --partition=CPU

set -e
cd ~/pace-bench/hpc_transfer

chmod +x stride startSolver
export STRIDE_SOLVER="./startSolver"
export STRIDE_TIMEOUT=1800
export STRIDE_GRACE=10
export STRIDE_DOWNLOADS_PATH=".stride-downloads"
mkdir -p "$STRIDE_DOWNLOADS_PATH"

./stride download -i instances.lst
./stride run -Oo -x trees-nodes-asc -i instances.lst