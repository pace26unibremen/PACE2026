#include "Graph/Forest.hpp"

#include <iostream>
#include <string>

using namespace std;
using namespace graph;

int main(int, char**)
{
    auto t = Forest(string(RES_DIR) + "examples/tree1.tree", 6, 1);
    t.dot(string(RES_DIR) + "examples/test2.dot");
    return 17;
}
