# Pace 2026

This is a project containing Solvers and Tree Representations 
for the Pace Challenge 2026.

## Build and run

To build the project **cmake** is required.

The project dependencies:

- **Boost** log and boost log_setup for the custom logger

- **Catch2** for the tests

- ~~**Cplex** for the ilp solver~~

    depricated

    Cplex can be downloaded and installed from the IBM website.
    The path to the installation of cplex and the architecture 
    should be defined in ```LocalConfig.cmake```. 
    The default values for a linux installation can be found in the template 
    ```LocalConfig.cmake.in``` and look like:
    ```cmake
    set(CPLEX_ROOT "/opt/ibm/ILOG/CPLEX_Studio2211" CACHE PATH "Path to CPLEX installation")
    set(CPLEX_ARCH "x86-64_linux" CACHE STRING "Architecture of CPLEX installation")
    ```

Cmake will download and install Catch2 locally if it can't be found.
Boost can be installed by any package manager.

Script to build and run main:
```sh
mkdir build
cd build
cmake ..
make main
./src/main 
```

## Logger

The project has a custom logger, based on the boost logger library.
A log message has channel (e.g. "Graph") and a severity (e.g. "Warning").

The log messages can be filtered by channel and severity.
The logs are written in std::clog and in a log file.

The filter and the log file can be specified:
```cpp
#include "Logger/Logger.hpp"

int main(int, char **)
{
    logFilterPath = "../log_filer.ini";
    logFilePath = "../.log";
    return 0
}
```
A filter file could be:
```
(%Channel%  != "Graph" and
 %Channel%  = "Solver" or
 %Severity% >= warning
)
```

## Tests

Run tests:

```sh
mkdir build
cd build
cmake ..
make
./tests/TreeIOTests
./tests/TreeEqualTests
./tests/TreeContractTests
```