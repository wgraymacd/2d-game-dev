#pragma once

#include "Skeleton.hpp"
#include "Bone.hpp"

// a single skeletal animation (e.g., run)
class SkelAnim
{
public:

    SkelAnim(const std::array<Skeleton, 10>& keyFrames)
        : m_keyFrames(keyFrames)
    { }

    /// @brief interpolate between two key frames to update current skeleton
    void update(float timeSinceLastKeyFrame)
    {
        float frac = timeSinceLastKeyFrame / m_frameDuration;
        size_t nextKeyFrameIdx = (m_lastKeyFrameIdx + 1) % m_keyFrames.size();

        const std::array<Bone, 7>& lastKeyFrameBones = m_keyFrames[m_lastKeyFrameIdx].getBones();
        const std::array<Bone, 7>& nextKeyFrameBones = m_keyFrames[nextKeyFrameIdx].getBones();
        std::array<Bone, 7>& currentFrameBones = m_currentFrame.getBones();

        for (size_t i = 0; i < currentFrameBones.size(); ++i)
        {
            currentFrameBones[i].pos = lastKeyFrameBones[i].pos + (nextKeyFrameBones[i].pos - lastKeyFrameBones[i].pos) * frac;
            currentFrameBones[i].angle = lastKeyFrameBones[i].angle + (nextKeyFrameBones[i].angle - lastKeyFrameBones[i].angle) * frac;
        }
    }

    Skeleton& getCurrentFrame()
    {
        return m_currentFrame;
    }

private:

    Skeleton m_currentFrame { { } }; // current skeleton
    std::array<Skeleton, 10> m_keyFrames;
    size_t m_lastKeyFrameIdx = 0;

    float m_frameDuration = 0.1f; // duration of each key frame (seconds)
};

namespace AnimConfig
{
    /*

    hip
        torso
            rightUpperArm
                rightForearm
                    rightHand
            leftUpperArm
                leftForearm
                    leftHand
            head
            rightThigh
                rightCalf
                    rightFoot
            leftThigh
                leftCalf
                    leftFoot

    Bone positioned by endpoint
    Rotations and positions defined relative to the bone's parent (heirarchical transformations, e.g., torso rotates, arms and legs rotate with it)
    Hip is root, position (0, 0), length 0

    Key frames store local transformations of each bone relative to its parent

    When transitioning to ragdoll (player killed), convert bones to rigid bodies, add joints, and apply forces

    */

    // Scale factor from asset dimensions to size in game (needed or do in game loop?)
    constexpr inline float scaleFactor = 1489.0f / 40.0f;

    // Data obtained from key frame parser in Godot 2D Skeletal Animation project
    constexpr std::array<Skeleton, 10> walkKeyFrames {
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, 0.0f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, 0.180659f, 0.0f },
            Bone { { 118.02f, 375.057f }, 0.0168346f, 0.0f },
            Bone { { -35.0191f, 380.023f }, .0689627f, 0.0f },
            Bone { { -57.0f, 42.0f }, 0.0494979f, 0.0f },
            Bone { { -72.0031f, 375.037f }, .148489f, 0.0f },
            Bone { { -104.009f, 366.036f }, 0.0504455f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -3.33337f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, -0.156167f, 0.0f },
            Bone { { 118.02f, 375.057f }, 0.174599f, 0.0f },
            Bone { { -35.0189f, 380.023f }, 0.044495f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.259675f, 0.0f },
            Bone { { -72.0027f, 375.036f }, 0.540573f, 0.0f },
            Bone { { -104.009f, 366.037f }, 0.0337172f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -6.66675f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, -0.0653241f, 0.0f },
            Bone { { 118.02f, 375.056f }, 0.239257f, 0.0f },
            Bone { { -35.0194f, 380.026f }, -0.107711f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.516687f, 0.0f },
            Bone { { -72.0026f, 375.034f }, 0.699089f, 0.0f },
            Bone { { -104.009f, 366.036f }, -0.0578607f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -10.0f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, 0.0824986f, 0.0f },
            Bone { { 118.02f, 375.056f }, 0.204228f, 0.0f },
            Bone { { -35.0198f, 380.027f }, -0.217013f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.597711f, 0.0f },
            Bone { { -72.0026f, 375.033f }, 0.582785f, 0.0f },
            Bone { { -104.009f, 366.037f }, -0.0110523f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -5.0f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, 0.249858f, 0.0f },
            Bone { { 118.02f, 375.056f }, 0.126722f, 0.0f },
            Bone { { -35.0192f, 380.027f }, -0.303159f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.634368f, 0.0f },
            Bone { { -72.0019f, 375.031f }, 0.389753f, 0.0f },
            Bone { { -104.009f, 366.038f }, 0.101466f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, 0.0f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, 0.509863f, 0.0f },
            Bone { { 118.019f, 375.054f }, -0.148831f, 0.0f },
            Bone { { -35.0186f, 380.025f }, -0.153771f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.563519f, 0.0f },
            Bone { { -72.0017f, 375.031f }, 1.74846e-07f, 0.0f },
            Bone { { -104.009f, 366.038f }, 0.351801f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -3.33337f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, 0.367252f, 0.0f },
            Bone { { 118.019f, 375.055f }, 0.217374f, 0.0f },
            Bone { { -35.0186f, 380.024f }, -0.158369f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.555166f, 0.0f },
            Bone { { -72.001f, 375.027f }, 0.180853f, 0.0f },
            Bone { { -104.009f, 366.038f }, 0.37414f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -6.66663f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, 0.123527f, 0.0f },
            Bone { { 118.019f, 375.056f }, 0.37529f, 0.0f },
            Bone { { -35.019f, 380.021f }, -0.23635f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.532798f, 0.0f },
            Bone { { -72.0007f, 375.026f }, 0.341373f, 0.0f },
            Bone { { -104.01f, 366.04f }, 0.206473f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -10.0f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, -0.0479234f, 0.0f },
            Bone { { 118.018f, 375.055f }, 0.376218f, 0.0f },
            Bone { { -35.0183f, 380.022f }, -0.215126f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.42017f, 0.0f },
            Bone { { -72.0016f, 375.031f }, 0.375566f, 0.0f },
            Bone { { -104.009f, 366.038f }, 0.0725748f, 0.0f }
        } },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -5.0f }, 0.0f, 0.0f },
            Bone { { 49.0f, 46.0f }, -0.172104f, 0.0f },
            Bone { { 118.019f, 375.056f }, 0.287437f, 0.0f },
            Bone { { -35.0189f, 380.024f }, -0.133179f, 0.0f },
            Bone { { -57.0f, 42.0f }, -0.282713f, 0.0f },
            Bone { { -72.0022f, 375.033f }, 0.360548f, 0.0f },
            Bone { { -104.01f, 366.04f }, -0.03879f, 0.0f }
        } }
    }; // ADS walk

    // constexpr std::array<Skeleton, 10> runKeyFrames {  }; // sprint, no ADS
    // constexpr std::array<Skeleton, 10> idleKeyFrames {  };
    // constexpr std::array<Skeleton, 10> crouchWalkKeyFrames {  };
    // constexpr std::array<Skeleton, 10> crouchRunKeyFrames {  };
    // constexpr std::array<Skeleton, 10> crouchIdleKeyFrames {  };
    // constexpr std::array<Skeleton, 10> jumpKeyFrames {  };
    // constexpr std::array<Skeleton, 10> fallKeyFrames {  };
    // constexpr std::array<Skeleton, 10> landKeyFrames {  };

    enum Index
    {
        HIP,
        LEFT_THIGH,
        LEFT_CALF,
        LEFT_FOOT,
        RIGHT_THIGH,
        RIGHT_CALF,
        RIGHT_FOOT,
        NUM_TYPES
    };
}
