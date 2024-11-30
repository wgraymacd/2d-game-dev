#pragma once

#include "Vec2.hpp"
#include <SFML/Graphics.hpp>

class Animation
{
    std::string m_name = "none";
	size_t m_frameCount = 1; // number of frames in the animation
	size_t m_frameDuration = 1; // length of one frame
	size_t m_currentFrame = 0; // current frame of the animation
	size_t m_framesPassed = 0; // number of game frames passed since created
	Vec2f m_size = { 0, 0 }; // dimensions of animation frame, sf::Textures are always dimensioned with unsigned integers
	sf::Sprite  m_sprite;

public:

    Animation() = default;

    Animation(const std::string& name, const sf::Texture& texture, size_t frameCount, size_t frameDuration)
        : m_name(name), m_sprite(texture), m_frameCount(frameCount), m_frameDuration(frameDuration)
    {
        m_size = Vec2f(texture.getSize().x / frameCount, texture.getSize().y);
        m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f);
        m_sprite.setTextureRect(sf::IntRect(m_currentFrame * m_size.x, 0, m_size.x, m_size.y));
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

    sf::Sprite& getSprite()
    {
        return m_sprite;
    }

    const std::string& getName() const
    {
        return m_name;
    }

    const Vec2f& getSize() const
    {
        return m_size;
    }
};