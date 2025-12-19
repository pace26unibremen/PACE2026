#ifndef PACE2026_INSTANCE_HPP
#define PACE2026_INSTANCE_HPP

#include "Forest.hpp"

namespace graph
{
    typedef std::vector<std::shared_ptr<Forest>> Instance;

    std::shared_ptr<Instance> ReadInstance(const std::filesystem::path& path);

    void WriteInstance(const std::shared_ptr<Instance>& instance, std::ostream& os);

    void WriteInstance(const std::shared_ptr<Instance>& instance, const std::filesystem::path& path);

    void DotInstance(const std::shared_ptr<Instance>& instance, std::ostream& os);

    void DotInstance(const std::shared_ptr<Instance>& instance, const std::filesystem::path& path);
}

#endif  //PACE2026_INSTANCE_HPP
