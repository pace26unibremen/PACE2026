# Pace 2026

This is a project containing Solvers and Tree Representations 
for the Pace Challenge 2026.

## Build and run

The project dependencies:

- **cmake** to build the project (minimum version `3.27.0`) 

- **Catch2** for the tests

Cmake will download and install Catch2 locally if it can't be found.

Script to build and run main:
```sh
mkdir build
cd build
cmake ..
make main
./src/main 
```

## Tests

Run tests:

```sh
mkdir build
cd build
cmake ..
make
./tests/ForestIOTests
./tests/TreeEqualTests
./tests/ForestRemoveEdgeTests
```