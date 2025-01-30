#pragma once

#include "Animation.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cassert>
#include <iostream>
#include <fstream>
#include <map>
#include <optional>

class Assets
{
    std::map<std::string, sf::Texture> m_textureMap;
    std::map<std::string, Animation> m_animationMap;
    std::map<std::string, sf::Font> m_fontMap;
    std::map<std::string, sf::SoundBuffer> m_soundBufferMap;
    std::map<std::string, std::optional<sf::Sound>> m_soundMap; // made this optional because of the lack of a default constructor
    /// TODO: could consider other methods for the sound map without std::optional, also check this file in general in light of optional usage, this was just a quick fix, haven't checked performance, maybe replace with std::unique_pointer (chatGPT rec)?

    /// @brief add a texture to the texture map
    void addTexture(const std::string& textureName, const std::string& path, bool smooth)
    {
        m_textureMap[textureName] = sf::Texture();

        if (!m_textureMap[textureName].loadFromFile(path))
        {
            std::cerr << "Could not load texture file: " << path << std::endl;
            m_textureMap.erase(textureName);
        }
        else
        {
            m_textureMap[textureName].setSmooth(smooth);
            std::cout << "Loaded texture: " << path << std::endl;
        }
    }

    // animations with their own texture file

    /// @brief add an animation to the animation map
    /// @param textureName texture used for animation, single file with frames from left to right equally spaced
    /// @param frameCount number of animation frames in the texture
    /// @param frameDuration number of game frames to maintain each animation frame
    void addAnimation(const std::string& animationName, const std::string& textureName, const int frameCount, const int frameDuration)
    {
        m_animationMap[animationName] = Animation(animationName, getTexture(textureName), frameCount, frameDuration);
    }

    /// @brief add a static animation to the animation map
    /// @param textureName texture atlas used
    /// @param position top left corner of texture region to be used
    /// @param size size in pixels of texture region
    void addAnimation(const std::string& animationName, const std::string& textureName, const Vec2i& position, const Vec2i& size)
    {
        m_animationMap[animationName] = Animation(animationName, getTexture(textureName), position, size);
    }

    /// @brief add a font to the font map
    void addFont(const std::string& fontName, const std::string& path)
    {
        m_fontMap[fontName] = sf::Font();
        if (!m_fontMap[fontName].openFromFile(path))
        {
            std::cerr << "Could not load font file: " << path << std::endl;
            m_fontMap.erase(fontName);
        }
        else
        {
            std::cout << "Loaded font: " << path << std::endl;
        }
    }

    /// @brief add a sound buffer and sound to the sound buffer and sound maps
    void addSound(const std::string& soundName, const std::string& path)
    {
        m_soundBufferMap[soundName] = sf::SoundBuffer();
        if (!m_soundBufferMap[soundName].loadFromFile(path))
        {
            std::cerr << "Could not load sound file: " << path << std::endl;
            m_soundBufferMap.erase(soundName);
        }
        else
        {
            std::cout << "Loaded sound: " << path << std::endl;
            m_soundMap[soundName] = sf::Sound(m_soundBufferMap[soundName]);
            // m_soundMap[soundName].value().setVolume(25);
        }
    }

public:
    /// @brief required for pre-allocation of entity memory pool component vectors
    Assets() = default;

    /// @brief loads all assets from asset configuration file
    /// @param path the file path to the asset configuration file
    void loadFromFile(const std::string& path)
    {
        std::ifstream file(path);
        std::string str;
        while (file.good())
        {
            file >> str;

            if (str == "Texture")
            {
                std::string name, path;
                file >> name >> path;
                addTexture(name, path, false);
            }
            else if (str == "Static")
            {
                std::string name, texture;
                Vec2i pos, size;
                file >> name >> texture >> pos.x >> pos.y >> size.x >> size.y;
                addAnimation(name, texture, pos, size);
            }
            else if (str == "Animation")
            {
                std::string name, texture;
                int frameCount, frameDuration;
                file >> name >> texture >> frameCount >> frameDuration;
                addAnimation(name, texture, frameCount, frameDuration);
            }
            else if (str == "Font")
            {
                std::string name, path;
                file >> name >> path;
                addFont(name, path);
            }
            else if (str == "Sound")
            {
                std::string name, path;
                file >> name >> path;
                addSound(name, path);
            }
            else
            {
                std::cerr << "Unknown asset type: " << str << std::endl;
            }
        }
        file.close();
    }

    const sf::Texture& getTexture(const std::string& textureName) const
    {
        assert(m_textureMap.find(textureName) != m_textureMap.end());
        return m_textureMap.at(textureName);
    }

    const Animation& getAnimation(const std::string& animationName) const
    {
        assert(m_animationMap.find(animationName) != m_animationMap.end());
        return m_animationMap.at(animationName);
    }

    const sf::Font& getFont(const std::string& fontName) const
    {
        assert(m_fontMap.find(fontName) != m_fontMap.end());
        return m_fontMap.at(fontName);
    }

    const std::map<std::string, sf::Texture>& getTextures() const
    {
        return m_textureMap;
    }

    const std::map<std::string, Animation>& getAnimations() const
    {
        return m_animationMap;
    }

    // sf::Sound& getSound(const std::string& soundName)
    // {
    //     assert(m_soundMap.find(soundName) != m_soundMap.end());
    //     return m_soundMap.at(soundName).value(); /// TODO: do I have to check to see if the sound at soundName has a value? test this
    // }

    void playSound(const std::string& soundName)
    {
        assert(m_soundMap.find(soundName) != m_soundMap.end());
        m_soundMap.at(soundName).value().play();
    }
};