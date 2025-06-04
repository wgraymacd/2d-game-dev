#pragma once

#include "Skeleton.hpp"
#include "Bone.hpp"

// a single skeletal animation (e.g., run)
class SkelAnim
{
public:

    SkelAnim() = default;

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

    Skeleton m_currentFrame; // current skeleton
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

    // Bone lengths

    // Sprite-to-bone offsets
    constexpr inline Vec2f leftThighOffset { 89.0f, 74.0f };
    constexpr inline Vec2f leftCalfOffset { 71.0f, 46.0f };
    constexpr inline Vec2f leftFootOffset { 67.0f, 40.0f };
    constexpr inline Vec2f rightThighOffset { 133.0f, 68.0f };
    constexpr inline Vec2f rightCalfOffset { 72.0f, 49.0f };
    constexpr inline Vec2f rightFootOffset { 67.0f, 40.0f };

    // Scale factor from asset dimensions to size in game (needed or do in game loop?)
    constexpr inline float scaleFactor = 200.0f / 1489.0f;

    // Data obtained from key frame parser in Godot 2D Skeletal Animation project
    /// TODO: not that the position here is actually worthless information, only rotations are happening here
    constexpr std::array<Skeleton, 10> walkKeyFrames {
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, 0.0f }, .0f },
            Bone { { 0.0f, 10.0f }, .197348f },
            Bone { { -0.000164032f, 390.023f }, .448043f },
            Bone { { -0.00108337f, 360.009f }, 0.16084f },
            Bone { { 0.0f, 10.0f }, 0.494897f },
            Bone { { -0.000206947f, 390.024f }, .361783f },
            Bone { { -0.000686646f, 360.019f }, 0.0459213f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -3.33333f }, 0.0f },
            Bone { { 0.0f, 10.0f }, 0.111758f },
            Bone { { -1.93119e-05f, 390.026f }, 0.697345f },
            Bone { { -0.000457764f, 360.01f }, -0.0574522f },
            Bone { { 0.0f, 10.0f }, -0.489455f },
            Bone { { -0.000134468f, 390.024f }, 0.642143f },
            Bone { { -0.000823975f, 360.021f }, -0.142197f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -6.66667f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.0537809f },
            Bone { { -1.13249e-05f, 390.024f }, 0.999743f },
            Bone { { -0.000610352f, 360.011f }, -0.418864f },
            Bone { { 0.0f, 10.0f }, -0.385112f },
            Bone { { -0.000248432f, 390.024f }, 0.731764f },
            Bone { { -0.000579834f, 360.022f }, -0.320333f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -10.0f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.363171f },
            Bone { { -9.48906e-05f, 390.025f }, 1.09372f },
            Bone { { -0.00134277f, 360.011f }, -0.46051f },
            Bone { { 0.0f, 10.0f }, -0.254857f },
            Bone { { -0.000245094f, 390.026f }, 0.773738f },
            Bone { { -0.000518799f, 360.023f }, -0.393167f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -5.0f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.551089f },
            Bone { { 2.76566e-05f, 390.024f }, 0.940123f },
            Bone { { -0.000488281f, 360.012f }, -0.378989f },
            Bone { { 0.0f, 10.0f }, -0.070217f },
            Bone { { -0.000306189f, 390.029f }, 0.6998f },
            Bone { { -0.000183105f, 360.023f }, -0.389152f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, 0.0f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.505232f },
            Bone { { 0.000161409f, 390.026f }, 0.370818f },
            Bone { { -0.000671387f, 360.013f }, -0.0869795f },
            Bone { { 0.0f, 10.0f }, 0.183315f },
            Bone { { -0.000389099f, 390.032f }, 0.466862f },
            Bone { { -0.000442505f, 360.024f }, -0.279619f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -3.33333f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.494937f },
            Bone { { 0.000713825f, 390.027f }, 0.647305f },
            Bone { { -0.000839233f, 360.014f }, -0.147325f },
            Bone { { 0.0f, 10.0f }, -0.00813705f },
            Bone { { -0.000392377f, 390.029f }, 0.878261f },
            Bone { { -0.000701904f, 360.024f }, -0.0231908f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -6.66667f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.385083f },
            Bone { { 0.000384331f, 390.028f }, 0.731711f },
            Bone { { -0.00109863f, 360.014f }, -0.320309f },
            Bone { { 0.0f, 10.0f }, -0.0537832f },
            Bone { { -0.00020504f, 390.028f }, 0.999744f },
            Bone { { -0.00100708f, 360.027f }, -0.295662f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -10.0f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.252402f },
            Bone { { 0.000225306f, 390.029f }, 0.772595f },
            Bone { { -0.000610352f, 360.015f }, -0.329856f },
            Bone { { 0.0f, 10.0f }, -0.359166f },
            Bone { { -0.000236511f, 390.031f }, 1.08924f },
            Bone { { -0.00143433f, 360.026f }, -0.331798f }
        }, scaleFactor },
        Skeleton { std::array<Bone, 7> {
            Bone { { 0.0f, -5.0f }, 0.0f },
            Bone { { 0.0f, 10.0f }, -0.063916f },
            Bone { { 0.000326931f, 390.028f }, 0.69425f },
            Bone { { -0.0012207f, 360.012f }, -0.285342f },
            Bone { { 0.0f, 10.0f }, -0.543931f },
            Bone { { -9.82285e-05f, 390.031f }, 0.933822f },
            Bone { { -0.000854492f, 360.026f }, -0.282387f }
        }, scaleFactor }
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
