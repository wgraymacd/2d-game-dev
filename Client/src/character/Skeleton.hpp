#pragma once

#include "Bone.hpp"

#include "physics/Vec2.hpp"

#include <vector>

// includes interpolation and key frame data
class Skeleton
{
public:

    Skeleton() = default;

    /// @todo look into the constexpr thing again later
    constexpr Skeleton(const std::array<Bone, 7>& bones, float scale)
    {
        for (size_t i = 0; i < bones.size(); ++i)
        {
            m_bones[i] = Bone { bones[i].pos * scale, bones[i].angle };
        }
    }

    std::array<Bone, 7>& getBones()
    {
        return m_bones;
    }

private:

    // at some time t, m_bones holds the exact positions of the bones of the skeleton (whether this is a key frame or an interpolation)
    std::array<Bone, 7> m_bones;
};
