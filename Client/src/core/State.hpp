// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// C++ standard libraries
#include <cstdint>

enum class State
{
    NONE,
    IDLE,
    RUN,
    WALK, // ADS + RUN input = WALK
    AIR,
    NUM_STATES
};
