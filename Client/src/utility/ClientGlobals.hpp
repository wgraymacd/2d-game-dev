// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Global
#include "Globals.hpp"

namespace Settings
{
    inline int windowSizeX = 1920; // default value, overriden by fullscreen mode, consider eliminating this variable (not constexpr)
    inline int windowSizeY = 1080; // default value, overriden by fullscreen mode, consider eliminating this variable (not constexpr)
    inline constexpr int frameRate = 120;
    inline constexpr int cellSizePixels = 10;
}

namespace Constants
{
    inline constexpr float pi = 3.14159265359f; // this rounds
}
