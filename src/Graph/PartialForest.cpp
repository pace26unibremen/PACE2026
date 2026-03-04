//
// Created by Philip Kail on 3/4/26.
//

#include "PartialForest.h"


graph::PartialForest::PartialForest(std::shared_ptr<std::vector<Node>> nodes,
                                    std::shared_ptr<std::unordered_map<Node*, unsigned int>> terminalToLabel,
                                    std::shared_ptr<std::unordered_map<unsigned int, Node*>> labelToTerminal,
                                    std::shared_ptr<std::vector<Node*>> roots, std::shared_ptr<Forest> fullForest,
                                    std::shared_ptr<Node> clusterPoint) :
        Forest(nodes,terminalToLabel,labelToTerminal,roots)
{}

void graph::PartialForest::decoupleForest() {}
void graph::PartialForest::coupleForest() {}
{}
