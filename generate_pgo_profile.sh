#!/usr/bin/env sh
###############################################################################
# generate_pgo_profile.sh — produce a PGO profile for the solver.
#
# Builds an instrumented startSolver, runs it over the training instances in
# res/profiler-instances/, and merges the result into a single .profdata file
# that the real build then consumes via -fprofile-use=<file>.
#
# The instrumented build is a plain build: a PGO profile is just branch/counter
# data keyed by function, so it is portable to the real (static / mimalloc)
# build regardless of how that one is linked.
#
#   usage: generate_pgo_profile.sh <output.profdata> [build_jobs]
#
# Requires: clang, cmake, ninja, the clang profile runtime (compiler-rt /
# libclang-rt-<ver>-dev) and llvm-profdata.
###############################################################################
set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
OUT="${1:?usage: generate_pgo_profile.sh <output.profdata> [build_jobs]}"
JOBS="${2:-$(nproc 2>/dev/null || echo 4)}"
INSTANCES="$HERE/res/profiler-instances"

PROFDATA=$(command -v llvm-profdata || ls /usr/bin/llvm-profdata* /usr/lib/llvm*/bin/llvm-profdata 2>/dev/null | head -1 || true)
[ -n "$PROFDATA" ] || { echo "generate_pgo_profile: llvm-profdata not found" >&2; exit 1; }

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# 1. Instrumented build (IR-based PGO instrumentation).
cmake -S "$HERE" -B "$WORK/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="-fprofile-generate" \
    -DCMAKE_EXE_LINKER_FLAGS="-fprofile-generate" >/dev/null
cmake --build "$WORK/build" --target startSolver -j "$JOBS" >/dev/null

# 2. Run the instrumented solver over each training instance (one profraw each).
mkdir -p "$WORK/prof"
i=0
for f in "$INSTANCES"/*.nw; do
    LLVM_PROFILE_FILE="$WORK/prof/p_${i}_%p.profraw" \
        "$WORK/build/src/startSolver" "$f" /dev/null >/dev/null 2>&1 || true
    i=$((i + 1))
done

# 3. Merge into the requested profile file.
"$PROFDATA" merge -o "$OUT" "$WORK"/prof/*.profraw
echo "generate_pgo_profile: wrote $OUT ($(wc -c < "$OUT") bytes) from $(ls "$INSTANCES"/*.nw | wc -l) instances"
