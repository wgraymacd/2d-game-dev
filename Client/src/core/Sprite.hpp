// // Copyright 2025, William MacDonald, All Rights Reserved.

// #pragma once

// // Physics
// #include "physics/Vec2.hpp"

// // External libraries
// #include <SFML/Graphics.hpp>

// // C++ standard libraries
// #include <string>

// class Sprite
// {
//     std::string m_name;
//     Vec2i m_atlasPosition; // pixels, position of sprite in texture atlas
//     Vec2i m_size; // pixels, dimensions of sprite
//     sf::Texture m_texture; // texture used for sprite
//     sf::Sprite m_sprite = sf::Sprite(m_texture);

// public:

//     Sprite(std::string_view name, const Vec2i& atlasPosition, const Vec2i& size, const sf::Texture& texture)
//         : m_name(name), m_atlasPosition(atlasPosition), m_size(size), m_texture(texture)
//     {
//         m_sprite.setOrigin(sf::Vector2f { size.x / 2.0f, size.y / 2.0f });
//         m_sprite.setTexture(texture);
//         m_sprite.setTextureRect(sf::IntRect({ atlasPosition.x, atlasPosition.y }, { size.x, size.y }));
//     }

//     sf::Sprite& getSprite()
//     {
//         return m_sprite;
//     }

//     const std::string& getName() const
//     {
//         return m_name;
//     }

//     const Vec2i& getSize() const
//     {
//         return m_size;
//     }
// };
