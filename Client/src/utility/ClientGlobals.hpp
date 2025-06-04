// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Global
#include "EntityBase.hpp"
#include "Globals.hpp"

// C++ Standard Library
#include <limits>

namespace Settings
{
    inline constexpr int worldMaxCellsX = 4000;
    inline constexpr int worldMaxCellsY = 1000;
    static_assert(4000 * 1000 < std::numeric_limits<int>::max(), "worldMaxCellsX * worldMaxCellsY must be less than the largest possible int");

    inline constexpr int cellSizePixels = 10;

    inline int windowSizeX = 1920; // default value, overriden by fullscreen mode, consider eliminating this variable (not constexpr)
    inline int windowSizeY = 1080; // default value, overriden by fullscreen mode, consider eliminating this variable (not constexpr)

    inline constexpr int frameRate = 120;
}

namespace Constants
{
    inline constexpr float pi = 3.14159265359f; // this rounds
}
