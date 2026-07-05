#!/usr/bin/env bash
###############################################################################
# docker_setup.sh — build the PACE 2026 MAF solver in a Debian 13.5 container.
#
# Per the PACE submission guidelines this script contains all steps necessary
# to build the solver from a clean Debian 13.5 (trixie) environment. Run it
# from the repository root:
#
#     ./docker_setup.sh
#
# Result: ./build/src/startSolver   (the competition solver; CI target name).
# Invoke as:  ./build/src/startSolver <input.nw> <output.nw>
#             or with no arguments to read stdin / write stdout.
###############################################################################
set -euo pipefail

# --- 1. Toolchain & dependencies -------------------------------------------
# clang is our reference compiler (C++23). ninja for fast builds, cmake >= 3.27.
# build-essential provides libstdc++ headers + linker that clang uses on glibc.
if [ "$(id -u)" -eq 0 ]; then SUDO=; else SUDO="sudo"; fi
export DEBIAN_FRONTEND=noninteractive
$SUDO apt-get update
$SUDO apt-get install -y --no-install-recommends \
    clang \
    cmake \
    ninja-build \
    build-essential \
    ca-certificates \
    libmimalloc-dev

# --- 2. Configure -----------------------------------------------------------
# Release + ThinLTO + mimalloc.
#
#   * Release enables -O3 — the optimization that actually matters.
#   * mimalloc is the big lever on top: the solver is allocation-bound (it copies
#     Forests heavily), and swapping glibc malloc for mimalloc measured
#     ~18-22% faster single-core on Debian 13, with identical optimal scores.
#     We link -lmimalloc so the loader interposes it over libc malloc (the
#     libmimalloc.so from libmimalloc-dev is present in this build/run image).
#   * -flto=thin (ThinLTO) does not change runtime speed measurably here, but it
#     shrinks the binary ~19% and costs nothing, so we keep it.
#   * We deliberately do NOT pass -march: the solver is branch/pointer bound,
#     not vectorizable, so -march gave no gain (and -march=x86-64-v3 regressed
#     ~10%), and an explicit arch could emit illegal instructions on unknown
#     judge hardware.
#
# BUILD_TESTING=OFF skips the Catch2 FetchContent so the build is fully offline
# and only produces the solver.
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DBUILD_TESTING=OFF \
    -DCMAKE_CXX_FLAGS="-flto=thin" \
    -DCMAKE_EXE_LINKER_FLAGS="-flto=thin -lmimalloc"

# --- 3. Build the solver ----------------------------------------------------
# BUILD_JOBS caps parallelism (defaults to all cores). Set it to leave the host
# some headroom, e.g. BUILD_JOBS=6 ./docker_setup.sh
cmake --build build --target startSolver -- -j "${BUILD_JOBS:-$(nproc)}"

echo
echo "Build complete: $(realpath build/src/startSolver)"
