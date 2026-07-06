#!/usr/bin/env bash
set -euo pipefail
echo "Initiating PGO Data Generation..."

cmake -S .. -B pgogenbuild -G Ninja -DCMAKE_BUILD_TYPE=PGOGEN -DCMAKE_CXX_FLAGS="-flto=thin" -DCMAKE_EXE_LINKER_FLAGS="-flto=thin"
cmake --build pgogenbuild --target startSolver


echo "Generating Raw Profiling Data..."

rm -f *.profraw default.profdata

i=1
for instance in Instances/*; do
    export LLVM_PROFILE_FILE="current.profraw";

    ./pgogenbuild/src/startSolver "$instance" /dev/null

    mv current.profraw "i${i}.profraw"
    ((i++))
done


echo "Mergin Profiling Data..."
llvm-profdata merge -output=default.profdata i*.profraw
echo "Moving Profiling Data to Project Root..."
mv default.profdata ../pgoData.profdata
rm -f *.profraw default.profdata

