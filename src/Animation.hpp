#pragma once

#include "Vec2.hpp"
#include <SFML/Graphics.hpp>
#include <string>

/// TODO: may want to consider a rework to separate textures and animations
class Animation
{
    std::string m_name = "none";
    int m_frameCount = 1;    // number of frames in the animation
    int m_frameDuration = 1; // length of one frame
    Vec2i m_atlasPosition = { 0, 0 };  // for regions of texture atlases
    Vec2i m_size;               // dimensions of animation frame

    int m_currentFrame = 0; // current frame of the animation
    unsigned int m_framesPassed = 0; // number of game frames passed since created, unsigned int doubles range of this value and has predictable wrap-around behavior, may have to use unsigned long if game is played for a long time
    sf::Texture m_texture; // texture used for sprite
    sf::Sprite m_sprite = sf::Sprite(m_texture);

    /// TODO: may be a better way to fix this defualt constructor problem than using a default texture and just replacing it

public:
    Animation() = default;

    // single image file with frames of animation equally spaced horizontally
    Animation(const std::string& name, const sf::Texture& texture, const int frameCount, const int frameDuration)
        : m_name(name),
        m_frameCount(frameCount),
        m_frameDuration(frameDuration),
        m_texture(texture)
    {
        m_size = Vec2i(m_texture.getSize().x / frameCount, m_texture.getSize().y);
        m_sprite.setOrigin(m_size.to<float>() / 2.0f);
        m_sprite.setTexture(texture);
        m_sprite.setTextureRect(sf::IntRect({ m_currentFrame * m_size.x, 0 }, m_size));
    }

    // region in texture atlas, not intended for use with update() or hasEnded()
    Animation(const std::string& name, const sf::Texture& texture, const Vec2i& atlasPosition, const Vec2i& size)
        : m_name(name),
        m_atlasPosition(atlasPosition),
        m_size(size),
        m_texture(texture)
    {
        m_sprite.setOrigin(m_size.to<float>() / 2.0f);
        m_sprite.setTexture(texture);
        m_sprite.setTextureRect(sf::IntRect({ atlasPosition.x, atlasPosition.y }, { size.x, size.y }));
    }

    /// @brief advances an animation by one frame
    // void update(sf::Sprite sprite)
    void update()
    {
        m_framesPassed++;
        m_currentFrame = (m_framesPassed / m_frameDuration) % m_frameCount; // if frameCount < (m_framesPassed / m_frameDuration), then this animation is looping
        m_sprite.setTextureRect(sf::IntRect({ m_currentFrame * m_size.x, 0 }, { m_size.x, m_size.y }));
    }

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

    const Vec2i& getSize() const
    {
        return m_size;
    }
};