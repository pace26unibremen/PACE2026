//
// Created by kaufm on 12.02.2026.
//

#include "ThreeTwoChainReductionRule.hpp"

#include <iostream>

solver::ThreeTwoChainReductionRule::ThreeTwoChainReductionRule(
    const std::shared_ptr<graph::Instance>& instance,
    const std::shared_ptr<Context>& context,
    std::vector<graph::Node*> nodes) :
        AbstractRule(instance, context, true)
{
    this->nodes = nodes;
}

solver::RuleReturnCode solver::ThreeTwoChainReductionRule::apply()
{
    if (this->isApplied)
    {
        throw std::invalid_argument("ThreeTwoChainReductionRule : apply : rule was already applied");
    }
    isApplied = true;


    return solver::RuleReturnCode::Continue;

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
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);

                            if (aNode->parent != nullptr && aNode->parent->parent != nullptr
                            && aNode->parent->parent->rightChild == cNode
                            && aNode->parent->rightChild != nullptr && aNode->parent->rightChild != cNode)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->rightChild);
                            }
                            else
                            {
                                checkForAllOtherTrees.push_back(false);
                                break;
                            }
                        }
                        if (allBoolsSayTrue(checkForAllOtherTrees) && bNodes.size() == instance->size())
                            return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);
                        else continue;


                    }
                    case 2:
                    {
                        auto bNode = node1->parent->rightChild;
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);

                            if (aNode->parent != nullptr && aNode->parent->parent != nullptr
                            && aNode->parent->parent->leftChild == cNode
                            && aNode->parent->leftChild != nullptr && aNode->parent->leftChild != aNode)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->leftChild);

                            }
                            else checkForAllOtherTrees.push_back(false);
                        }
                            if (allBoolsSayTrue(checkForAllOtherTrees))
                                return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);

                    }
                    case 3:
                    {
                        auto bNode = node1->parent->rightChild->leftChild;
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);

                            if (aNode->parent != nullptr && aNode->parent->rightChild != nullptr
                            && aNode->parent->rightChild != aNode
                            && aNode->parent->rightChild->rightChild == cNode
                            && aNode->parent->rightChild->leftChild != nullptr)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->rightChild->leftChild);
                            }
                            else
                            {
                                checkForAllOtherTrees.push_back(false);
                                break;
                            }
                        }
                        if (allBoolsSayTrue(checkForAllOtherTrees) && bNodes.size() == instance->size())
                            return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);
                        else continue;
                    }
                    case 4:
                    {
                        auto bNode = node1->parent->rightChild->rightChild;
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);

                            if (aNode->parent != nullptr && aNode->parent->rightChild != nullptr
                            && aNode->parent->rightChild != aNode
                            && aNode->parent->rightChild->leftChild == cNode
                            && aNode->parent->rightChild->rightChild != nullptr)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->rightChild->rightChild);
                            }
                            else
                            {
                                checkForAllOtherTrees.push_back(false);
                                break;
                            }
                        }
                        if (allBoolsSayTrue(checkForAllOtherTrees) && bNodes.size() == instance->size())
                            return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);
                        else continue;
                    }
                    case 5:
                    {
                        auto bNode = node1->parent->leftChild->rightChild;
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);

                            if (aNode->parent != nullptr && aNode->parent->leftChild != nullptr
                            && aNode->parent->leftChild != aNode
                            && aNode->parent->leftChild->leftChild == cNode
                            && aNode->parent->leftChild->rightChild != nullptr)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->leftChild->rightChild);
                            }
                            else
                            {
                                checkForAllOtherTrees.push_back(false);
                                break;
                            }
                        }

                        if (allBoolsSayTrue(checkForAllOtherTrees) && bNodes.size() == instance->size())
                            return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);
                        else continue;
                    }
                    case 6:
                    {
                        auto bNode = node1->parent->leftChild->leftChild;
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);
                            if (aNode->parent != nullptr && aNode->parent->leftChild != nullptr
                            && aNode->parent->leftChild != aNode
                            && aNode->parent->leftChild->leftChild != nullptr
                            && aNode->parent->leftChild->rightChild == cNode)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->leftChild->leftChild);
                            }
                            else
                            {
                                checkForAllOtherTrees.push_back(false);
                                break;
                            }
                        }
                        if (allBoolsSayTrue(checkForAllOtherTrees) && bNodes.size() == instance->size())
                            return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);
                        else continue;
                    }
                    case 7:
                    {
                        auto bNode =  node1->parent->leftChild;
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);

                            if (aNode->parent !=nullptr && aNode->parent->parent != nullptr
                            && aNode->parent->rightChild != aNode
                            && aNode->parent->parent->leftChild == cNode
                            && aNode->parent->leftChild != nullptr)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->leftChild);
                            }
                            else
                            {
                                checkForAllOtherTrees.push_back(false);
                                break;
                            }
                        }
                        if (allBoolsSayTrue(checkForAllOtherTrees) && bNodes.size() == instance->size())
                            return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);
                        else continue;
                    }
                    case 8:
                    {
                        auto bNode = node1->parent->rightChild;
                        std::vector<graph::Node*> bNodes = {bNode};
                        std::vector<bool> checkForAllOtherTrees = {};
                        checkForAllOtherTrees.reserve(instance->size()-1);
                        bNodes.reserve(instance->size());

                        for (int i = 1; i < instance->size(); i++)
                        {
                            auto fi = instance->at(i);
                            auto aNode = fi->LabelToTerminal().at(label1);
                            auto cNode = fi->LabelToTerminal().at(label2);
                            if (aNode->parent !=nullptr && aNode->parent->parent != nullptr
                            && aNode->parent->leftChild != aNode
                            && aNode->parent->parent->rightChild == cNode
                            && aNode->parent->rightChild != nullptr)
                            {
                                checkForAllOtherTrees.push_back(true);
                                bNodes.push_back(aNode->parent->rightChild);
                            }
                            else
                            {
                                checkForAllOtherTrees.push_back(false);
                                break;
                            }
                        }
                        if (allBoolsSayTrue(checkForAllOtherTrees) && bNodes.size() == instance->size())
                            return std::make_shared<ThreeTwoChainReductionRule>(instance, context, bNodes);
                        else continue;
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
