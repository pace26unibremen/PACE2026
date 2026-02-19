#include "Instance.hpp"

#include <fstream>
#include <vector>
#include <stdexcept>
#include <regex>

#include "Forest.hpp"
#include "ForestIO.hpp"

std::shared_ptr<graph::Instance> graph::ReadInstance(const std::filesystem::path& path)
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

    auto result = std::make_shared<Instance>();

    int numberOfForests = -1;
    int numberOfTerminals = -1;

    static const std::regex regex_header = std::regex(R"(#p\s(\d+)\s(\d+)\s*)");
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
        result->reserve(numberOfForests);
        for(int i = 0; i < numberOfForests; i++)
        {
            auto fi_ptr = std::make_shared<Forest>(ForestIO::ReadNewick(file, numberOfTerminals));
            result->emplace_back(fi_ptr);
        }
    }
    else
    {
        throw std::invalid_argument("Instance : no header expression \"#p {t} {n}\"");
    }
    return result;
}

void graph::WriteInstance(const std::shared_ptr<Instance>& instance, std::ostream& os)
{
    os << "#p " << instance->size() << " ";
    if (not instance->empty())
    {
        os << instance->at(0)->TerminalToLabel().size();
    }
    else
    {
        os << " ";
    }
    os << "\n";

    for(const auto& forest : *instance)
    {
        os << "# ==================== #\n";
        forest->write(os);
    }
}

void graph::WriteInstance(const std::shared_ptr<Instance>& instance, const std::filesystem::path& path)
{
    std::ofstream outStream(path);
    if (!outStream.is_open())
    {
        throw std::invalid_argument("Instance : WriteInstance : couldn't open file");
    }
    WriteInstance(instance ,outStream);
    outStream.close();
}

void graph::DotInstance(const std::shared_ptr<Instance>& instance, std::ostream& os)
{
    os << "digraph Instance {\n"
       << "splines = false\n\n";

    std::string subgraphParams =
       "style=dotted\n;"
       "node [\n"
       "    shape = circle,\n"
       "    fontsize = 15,\n"
       "    label = \"\",\n"
       "    height = 0.1,\n"
       "    fillcolor = \"#00000022\",\n"
       "    style = filled,\n"
       "    fixedsize = true,\n"
       "    labelloc = t];\n"
       "edge [arrowhead = none];\n\n";

    for (unsigned int i = 0; i < instance->size(); ++i)
    {
        ForestIO::WriteDotSubgraph(*instance->at(i), os, subgraphParams);
    }

    os << "}" << std::endl;
}

void graph::DotInstance(const std::shared_ptr<Instance>& instance, const std::filesystem::path& path)
{
    std::ofstream outStream(path);
    if (!outStream.is_open())
    {
        throw std::invalid_argument("Instance : DotInstance : couldn't open file");
    }
    DotInstance(instance ,outStream);
    outStream.close();
}
