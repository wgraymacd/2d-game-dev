#pragma once

#include "Bone.hpp"

#include "physics/Vec2.hpp"

#include <vector>

// includes interpolation and key frame data
class Skeleton
{
public:

    constexpr Skeleton(const std::array<Bone, 7>& bones)
        : m_bones(bones)
    { }

    std::array<Bone, 7>& getBones()
    {
        return m_bones;
    }

private:

    // at some time t, m_bones holds the exact positions of the bones of the skeleton (whether this is a key frame or an interpolation)
    std::array<Bone, 7> m_bones;
};
