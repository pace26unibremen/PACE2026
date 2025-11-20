#ifndef PACE2026_INSTANCE_HPP
#define PACE2026_INSTANCE_HPP

#include "Forest.hpp"

namespace graph
{
    typedef std::vector<Forest> Instance;

    Instance ReadInstance(const std::filesystem::path& path);
}

#endif  //PACE2026_INSTANCE_HPP
