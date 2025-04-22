// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Physics
#include "physics/Vec2.hpp"

// External libraries
#include <SFML/Graphics.hpp>

// C++ standard libraries
#include <string>

class Animation
{
    std::string m_name;
    int m_frameCount; // number of animation frames in the animation
    int m_frameDuration; // number of game frames in one animation frame
    Vec2i m_atlasPosition; // for regions of texture atlases
    Vec2i m_size; // dimensions of one animation frame

    int m_currentFrame; // current frame of the animation
    int m_gameFramesPassed; // number of game frames passed since created
    sf::Texture m_texture; // texture used for sprite
    sf::Sprite m_sprite = sf::Sprite(m_texture);
    /// TODO: may be a better way to fix this defualt constructor problem than using a default texture and just replacing it

public:
    /// @brief default constructor needed for memory pool initialization (Animation part of CAnimation)
    Animation() = default;

    /// @brief construct animation with a single image file with frames of animation equally spaced horizontally
    Animation(const std::string& name, const sf::Texture& texture, int frameCount, int frameDuration)
        : m_name(name), m_frameCount(frameCount), m_frameDuration(frameDuration), m_texture(texture)
    {
        m_size = { static_cast<int>(m_texture.getSize().x), static_cast<int>(m_texture.getSize().y) };
        m_sprite.setOrigin(sf::Vector2f { m_size.x / 2.0f, m_size.y / 2.0f });
        m_sprite.setTexture(texture);
        m_sprite.setTextureRect(sf::IntRect { { m_currentFrame * m_size.x, 0 }, { m_size.x, m_size.y } });
    }

    /// @brief construct a "static" animation with only 1 frame from a region in a texture atlas
    Animation(const std::string& name, const sf::Texture& texture, const Vec2i& atlasPosition, const Vec2i& size)
        : m_name(name), m_atlasPosition(atlasPosition), m_size(size), m_texture(texture)
    {
        m_sprite.setOrigin({ size.x / 2.0f, size.y / 2.0f });
        m_sprite.setTexture(texture);
        m_sprite.setTextureRect(sf::IntRect({ atlasPosition.x, atlasPosition.y }, { size.x, size.y }));
    }

    /// @brief advances an animation by one frame
    void updateLoop()
    {
        m_gameFramesPassed++;
        m_currentFrame = (m_gameFramesPassed / m_frameDuration) % m_frameCount; // if frameCount < (m_gameFramesPassed / m_frameDuration), then this animation is looping
        m_sprite.setTextureRect(sf::IntRect({ m_currentFrame * m_size.x, 0 }, { m_size.x, m_size.y }));
    }

    /// @brief determines if a non-looping animation has ended
    bool hasEnded() const
    {
        return (m_gameFramesPassed / m_frameDuration > m_currentFrame);
    }

    sf::Sprite& getSprite()
    {
        return m_sprite;
    }

    const std::string& getName() const
    {
        return m_name;
    }

    const Vec2i& getSize() const
    {
        return m_size;
    }
};
