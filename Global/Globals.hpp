// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <limits>
#include <cstddef>

using EntityID = unsigned int;

namespace Settings
{
    inline constexpr EntityID worldMaxEntities = 1000; // not including tiles or other things outside of main entity memory pool
    inline constexpr size_t worldMaxCellsX = 4000;
    inline constexpr size_t worldMaxCellsY = 1000;

    static_assert(4000 * 1000 < std::numeric_limits<int>::max(), "worldMaxCellsX * worldMaxCellsY must be less than the largest possible int");
}
