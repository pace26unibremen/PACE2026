#include "RuleStatsPlugin.hpp"

#include <utility>

solver::plugin::RuleStatsPlugin::RuleStatsPlugin(std::shared_ptr<MetricsCollector> collector,
                                                  bool collectTiming)
    : collector(std::move(collector)), collectTiming(collectTiming)
{
}

void solver::plugin::RuleStatsPlugin::beforeApply(const std::shared_ptr<solver::AbstractRule>&)
{
    if (collectTiming)
        ruleStart = std::chrono::steady_clock::now();
}

void solver::plugin::RuleStatsPlugin::onApply(const std::shared_ptr<solver::AbstractRule>& rule)
{
    const std::string& ruleName = rule->name();
    collector->ruleCounts[ruleName]++;
    if (collectTiming)
    {
        using namespace std::chrono;
        const double elapsedMs = duration<double, std::milli>(steady_clock::now() - ruleStart).count();
        collector->ruleTimes_ms[ruleName] += elapsedMs;
    }
}

void solver::plugin::RuleStatsPlugin::onEnd()
{
    emitStrideLine("rule_times_ms", toJson(collector->ruleTimes_ms));
    emitStrideLine("rule_counts", toJson(collector->ruleCounts));
}
