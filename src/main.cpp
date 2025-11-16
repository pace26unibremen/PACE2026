#include "Graph/Forest.hpp"

#include <iostream>
#include <string>

using namespace std;
using namespace graph;

int main(int, char**)
{
    auto t = Forest(string(RES_DIR) + "examples/tree1.tree");
    auto t2 = t.copy();
    cout<<(t == t2)<<endl;
    t2.removeEdge(5);
    cout<<(t == t2)<<endl;
    t.dot(string(RES_DIR) + "examples/test2.dot");
    return 17;
}
