//
// Created by kaufm on 12.02.2026.
//

#include "ThreeTwoChainReductionRule.hpp"

#include <iostream>

solver::ThreeTwoChainReductionRule::ThreeTwoChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context,
    graph::Node* node) :
        AbstractRule(instance, context, true)
{
    this->node = node;
}

solver::RuleReturnCode solver::ThreeTwoChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;

}

void solver::ThreeTwoChainReductionRule::unapply()
{
    if (not this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : unapply : rule is not applied");
    }
    isApplied = false;

}

bool solver::ThreeTwoChainReductionRule::allBoolsSayTrue(std::vector<bool> list)
{
    for (const auto& item : list)
    {
        if (not item) return false;
    }
    return true;
}

bool solver::ThreeTwoChainReductionRule::checkIfContainedInRoot(graph::Node* node, std::shared_ptr<graph::Forest> forest)
{
    for (int i = 0; i < forest->Roots().size(); i++)
    {
        if (forest->Roots().at(i) == node) return true;
    }
    return false;
}

int solver::ThreeTwoChainReductionRule::bCheck(graph::Node* node1, graph::Node* node2, std::shared_ptr<graph::Forest> forest)
{
    auto listOfNodes = forest->Nodes();
    auto terminals = forest->TerminalToLabel();

    if (node1->parent != nullptr && node1->parent->parent != nullptr
        && node1->parent->parent->rightChild == node2
        && node1->parent->rightChild != nullptr && node1->parent->rightChild != node1 ) return 1;

    if (node1->parent != nullptr && node1->parent->parent != nullptr
        && node1->parent->parent->leftChild == node2
        && node1->parent->leftChild != nullptr && node1->parent->leftChild != node1) return 2;

    if (node1->parent != nullptr && node1->parent->rightChild != nullptr && node1->parent->rightChild != node1
        && node1->parent->rightChild->rightChild == node2
        && node1->parent->rightChild->leftChild != nullptr) return 3;

    if (node1->parent != nullptr && node1->parent->rightChild != nullptr && node1->parent->rightChild != node1
        && node1->parent->rightChild->leftChild == node2
        && node1->parent->rightChild->rightChild != nullptr) return 4;

    if (node1->parent != nullptr && node1->parent->leftChild != nullptr && node1->parent->leftChild != node1
        && node1->parent->leftChild->leftChild == node2 && node1->parent->leftChild->rightChild != nullptr) return 5;

    if (node1->parent != nullptr && node1->parent->leftChild != nullptr && node1->parent->leftChild != node1
        && node1->parent->leftChild->leftChild != nullptr && node1->parent->leftChild->rightChild == node2) return 6;

    if (node1->parent !=nullptr && node1->parent->parent != nullptr && node1->parent->rightChild != node1
        && node1->parent->parent->leftChild == node2 && node1->parent->leftChild != nullptr) return 7;

    if (node1->parent !=nullptr && node1->parent->parent != nullptr && node1->parent->leftChild != node1
        && node1->parent->parent->rightChild == node2 && node1->parent->rightChild != nullptr) return 8;

    return 0;
}

std::shared_ptr<solver::AbstractRule>
solver::ThreeTwoChainReductionRule::isApplicable(const std::shared_ptr<graph::Instance>& instance,
                                                 const std::shared_ptr<Context>& context)
{
    auto f = instance->at(0);
    for (const auto& [label1,node1] : f->LabelToTerminal())
    {
        for (const auto& [label2,node2] : f->LabelToTerminal())
        {
            //Nodes arent the same and they arent pendant to each other.
            if (label1 != label2 && node1->sibling != node2 && node2->sibling != node1
            //And they arent root nodes
            && not checkIfContainedInRoot(node1,f) && not checkIfContainedInRoot(node2,f)
            // The nodes follow the B schema: Only one pendant node between them.
            )
            {
                auto bCase = bCheck(node1, node2, f);

                switch (bCase)
                {
                    case 0:
                    {
                        continue;
                    }
                    case 1:
                    {
                        auto bNode = node1->parent->rightChild;
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            if (node1->parent != nullptr && node1->parent->parent != nullptr
                            && node1->parent->parent->rightChild == node2
                            && node1->parent->rightChild == node2 && node1->parent->rightChild != node1)
                                checkForAllOtherTrees.push_back(true);
                            else checkForAllOtherTrees.push_back(false);

                            if (allBoolsSayTrue(checkForAllOtherTrees))
                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNode);
                        }


                    }
                    case 2:
                    {
                        auto bNode = node1->parent->leftChild;
                    }
                    case 3:
                    {
                        auto bNode = node1->parent->rightChild->leftChild;
                    }
                    case 4:
                    {
                        auto bNode = node1->parent->rightChild->rightChild;
                    }
                    case 5:
                    {
                        auto bNode = node1->parent->leftChild->rightChild;
                    }
                    case 6:
                    {
                        auto bNode = node1->parent->leftChild->leftChild;
                    }
                    case 7:
                    {
                        auto bNode =  node1->parent->leftChild;
                    }
                    case 8:
                    {
                        auto bNode = node1->parent->rightChild;
                    }
                    default:
                    {
                        throw std::invalid_argument("ThreeTwoChainReductionRule : default case in isApplicable reached "
                                                    "-how the hell did this happen");
                    }
                }
            }
        }
    }

    return nullptr;
}

std::string solver::ThreeTwoChainReductionRule::name() const
{
    return "ThreeTwoChainReductionRule";
}
