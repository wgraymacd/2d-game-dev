/// NOTE: using constexp ensures that the condition is checked at compile time, branch that isn't valid is discarded, saves runtime processing

#pragma once

#include "Vec2.hpp"

// usings
using EntityID = unsigned long;
// Vec2f, Vec2i, and Vec2ui in Vec2.hpp

// settings
namespace GlobalSettings
{
    // inline: allows vars to have external linkage (accessible across multiple translation units) but avoids the One Definition Rule (ODR) violation by ensuring they are defined only once, ensures that multiple inclusions of this class do not result in duplicate definitions of the variables
    // static: ensures the variables have internal linkage, meaning they are private to the translation unit (source file) in which they are declared, prevents the variables from being redefined in multiple files if the header is included in more than one source file, not needed since inline takes precedence here
    // constexpr: constant, hardcoded, known at compile time, never changes

    inline Vec2ui windowSize = { 1280, 720 }; // default value, overriden by fullscreen mode
    inline const int frameRate = 120;
    inline const Vec2ui worldMaxCells = { 500, 250 };
    inline const Vec2ui cellSizePixels = { 10, 10 };
    inline const EntityID worldMaxEntities = 1000; // not including tiles or other things outside of main entity memory pool
}