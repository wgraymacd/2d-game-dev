// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Global
#include "EntityBase.hpp"

// C++ Standard Library
#include <limits>

namespace Settings
{
    inline constexpr EntityID worldMaxEntities = 1000; // not including tiles or other things outside of main entity memory pool
}
