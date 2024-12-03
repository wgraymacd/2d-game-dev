#pragma once

#include "Vec2.hpp"
#include <SFML/Graphics.hpp>

class Animation
{
    std::string m_name = "none";
    size_t m_frameCount = 1; // number of frames in the animation
    size_t m_frameDuration = 1; // length of one frame
    Vec2f m_position = {0, 0}; // for regions of texture atlases
    Vec2f m_size; // dimensions of animation frame

    size_t m_currentFrame = 0; // current frame of the animation
    size_t m_framesPassed = 0; // number of game frames passed since created
    sf::Sprite m_sprite;

public:
    Animation() = default;

    // single image file with frames of animation equally spaced horizontally
    Animation(const std::string &name, const sf::Texture &texture, const size_t frameCount, const size_t frameDuration)
        : m_name(name), m_sprite(texture), m_frameCount(frameCount), m_frameDuration(frameDuration)
    {
        m_sprite.setOrigin(m_size / 2.0f);
        m_size = Vec2f(texture.getSize().x / frameCount, texture.getSize().y);
        m_sprite.setTextureRect(sf::IntRect(m_currentFrame * m_size.x, 0, m_size.x, m_size.y));
    }

    // region in texture atlas, not intended for use with update() or hasEnded()
    Animation(const std::string &name, const sf::Texture &texture, const Vec2f &position, const Vec2f &size)
        : m_name(name), m_sprite(texture), m_position(position), m_size(size)
    {
        m_sprite.setOrigin(m_size / 2.0f);
        m_sprite.setTextureRect(sf::IntRect(position.x, position.y, size.x, size.y));
    }

    // advance the animation by one frame
    void update()
    {
        m_framesPassed++;
        m_currentFrame = (m_framesPassed / m_frameDuration) % m_frameCount; // if frameCount < (m_framesPassed / m_frameDuration), then this animation is looping
        m_sprite.setTextureRect(sf::IntRect(m_currentFrame * m_size.x, 0, m_size.x, m_size.y));
    }

    // determine if one loop of the animation has ended
    bool hasEnded() const
    {
        return (m_framesPassed > m_currentFrame);
    }

    sf::Sprite &getSprite()
    {
        return m_sprite;
    }

    const std::string &getName() const
    {
        return m_name;
    }

    const Vec2f &getSize() const
    {
        return m_size;
    }
};