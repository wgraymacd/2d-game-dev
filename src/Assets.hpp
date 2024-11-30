#pragma once

#include "Animation.hpp"
#include <SFML/Graphics.hpp>

#include <cassert>
#include <iostream>
#include <fstream>

class Assets
{
    std::map<std::string, sf::Texture> m_textureMap;
    std::map<std::string, Animation> m_animationMap;
    std::map<std::string, sf::Font> m_fontMap;

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

    void addAnimation(const std::string& animationName, const std::string& textureName, const size_t frameCount, const size_t frameDuration)
    {
        const sf::Texture& texture = getTexture(textureName);
        m_animationMap[animationName] = Animation(animationName, texture, frameCount, frameDuration);
    }

    void addFont(const std::string& fontName, const std::string& path)
    {
        m_fontMap[fontName] = sf::Font();
        if (!m_fontMap[fontName].loadFromFile(path))
        {
            std::cerr << "Could not load font file: " << path << std::endl;
            m_fontMap.erase(fontName);
        }
        else
        {
            std::cout << "Loaded font: " << path << std::endl;
        }

    }

public:

    Assets() = default;

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
            else if (str == "Animation")
            {
                std::string name, texture;
                size_t frameCount, frameDuration;
                file >> name >> texture >> frameCount >> frameDuration;
                addAnimation(name, texture, frameCount, frameDuration);
            }
            else if (str == "Font")
            {
                std::string name, path;
                file >> name >> path;
                addFont(name, path);
            }
            else
            {
                std::cerr << "Unknown asset type: " << str << std::endl;
            }
        }
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
};