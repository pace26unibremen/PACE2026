#include "AbstractPlugin.hpp"

// Default no-op implementations. Subclasses override only the hooks they need.

void solver::plugin::AbstractPlugin::init(const std::shared_ptr<graph::Instance>&) {}

void solver::plugin::AbstractPlugin::beforeApply(const std::shared_ptr<solver::AbstractRule>&) {}

void solver::plugin::AbstractPlugin::onApply(const std::shared_ptr<solver::AbstractRule>&) {}

void solver::plugin::AbstractPlugin::onUnapply(const std::shared_ptr<solver::AbstractRule>&) {}

void solver::plugin::AbstractPlugin::onTempUnapply(const std::shared_ptr<solver::AbstractRule>&, bool) {}

void solver::plugin::AbstractPlugin::onTempApply(const std::shared_ptr<solver::AbstractRule>&) {}

void solver::plugin::AbstractPlugin::onNewBestSolution(std::size_t) {}

void solver::plugin::AbstractPlugin::onEnd() {}
