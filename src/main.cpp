#include "Graph/Tree.hpp"

#include <iostream>
#include <string>

using namespace std;
using namespace graph;

int main(int, char**)
{
    auto t = Tree(string(RES_DIR) + "examples/test2.tree");


//    t.print();
    t.dot(cout);
    return 17;
}
