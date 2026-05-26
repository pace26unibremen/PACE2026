#include "AbstractStridePlugin.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{

/// Format a double with exactly 3 decimal places, no trailing exponent notation.
std::string formatDouble(double v)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << v;
    return ss.str();
}

} // anonymous namespace

std::string solver::plugin::AbstractStridePlugin::toJson(const std::map<std::string, int>& m)
{
    std::string out = "{";
    bool first = true;
    for (const auto& [k, v] : m)
    {
        if (!first) out += ',';
        out += '"';
        out += k;
        out += "\":";
        out += std::to_string(v);
        first = false;
    }
    out += '}';
    return out;
}

std::string solver::plugin::AbstractStridePlugin::toJson(const std::map<std::string, double>& m)
{
    std::string out = "{";
    bool first = true;
    for (const auto& [k, v] : m)
    {
        if (!first) out += ',';
        out += '"';
        out += k;
        out += "\":";
        out += formatDouble(v);
        first = false;
    }
    out += '}';
    return out;
}

std::string solver::plugin::AbstractStridePlugin::toJson(const std::vector<Snapshot>& snapshots)
{
    std::string out = "[";
    bool first = true;
    for (const auto& s : snapshots)
    {
        if (!first) out += ',';
        out += "{\"wtime\":";
        out += formatDouble(s.wtime);
        out += ",\"score\":";
        out += std::to_string(s.score);
        out += ",\"branch_opens\":";
        out += std::to_string(s.branchOpens);
        out += ",\"branch_closes\":";
        out += std::to_string(s.branchCloses);
        out += ",\"rule_counts\":";
        out += toJson(s.ruleCounts);
        out += ",\"rule_times_ms\":";
        out += toJson(s.ruleTimes_ms);
        out += '}';
        first = false;
    }
    out += ']';
    return out;
}

void solver::plugin::AbstractStridePlugin::emitStrideLine(const std::string& key, const std::string& jsonValue)
{
    std::cout << "#s " << key << ' ' << jsonValue << '\n';
}
