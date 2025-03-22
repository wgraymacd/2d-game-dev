// Copyright 2025, William MacDonald, All Rights Reserved.

/// NOTE: using constexp ensures that the condition is checked at compile time, branch that isn't valid is discarded, saves runtime processing

#pragma once

// usings
using EntityID = int; /// TODO: worry about whether I make this int or something else when worrying about all data type stuff
// Vec2f, Vec2i, etc. in Vec2.hpp

// settings
namespace GlobalSettings {
    /// TODO: change these types to minimum possible storage with uint16_t and whatever
    inline int windowSizeX = 1920; // default value, overriden by fullscreen mode
    inline int windowSizeY = 1080; // default value, overriden by fullscreen mode
    inline constexpr int frameRate = 120;
    inline constexpr int worldMaxCellsX = 4000;
    inline constexpr int worldMaxCellsY = 1000;
    inline constexpr int cellSizePixels = 10;
    inline constexpr EntityID worldMaxEntities = 1000; // not including tiles or other things outside of main entity memory pool
}
