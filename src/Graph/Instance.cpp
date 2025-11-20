#include "Instance.hpp"

#include <fstream>
#include <vector>
#include <stdexcept>
#include <regex>

#include "Forest.hpp"
#include "ForestIO.hpp"

graph::Instance graph::ReadInstance(const std::filesystem::path& path)
{
    if (path.empty())
    {
        throw std::invalid_argument("Instance : ReadInstance : provided file path is empty");
    }
    std::ifstream file = std::ifstream(path);
    if (!file.is_open())
    {
        throw std::invalid_argument("Instance : ReadInstance : unable to open file");
    }

    Instance result;

    int numberOfForests = -1;
    int numberOfTerminals = -1;

    static const std::regex regex_header = std::regex(R"(#p\s(\d+)\s(\d+))");
    std::smatch match;
    std::string line;
    while(getline(file, line))
    {
        if (std::regex_match(line, match, regex_header))
        {
            numberOfForests = std::stoi(match[1]);
            numberOfTerminals = std::stoi(match[2]);
            break;
        }
    }

    if(numberOfForests >= 0)
    {
        result.reserve(numberOfForests);
        for(int i = 0; i < numberOfForests; i++)
        {
            Forest fi = ForestIO::ReadNewick(file, numberOfTerminals);
            fi.sortChildrenAndCollectTerminals();
            result.emplace_back(fi);
        }
    }
    else
    {
        throw std::invalid_argument("Instance : no header expression \"#p {t} {n}\"");
    }
    return result;
}
