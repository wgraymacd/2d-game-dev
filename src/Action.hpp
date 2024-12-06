#pragma once

#include "Vec2.hpp"

#include <string>

class Action
{
    std::string m_name = "NONE";
    std::string m_type = "NONE";
    Vec2i m_position; // mouse click position

public:

    Action() = default;

    Action(const std::string& name, const std::string& type)
        : m_name(name), m_type(type) { }

    Action(const std::string& name, const std::string& type, const Vec2i &pos)
        : m_name(name), m_type(type), m_position(pos) { }

    const std::string& name() const
    {
        return m_name;
    }

    const std::string& type() const
    {
        return m_type;
    }

    Vec2i pos() const
    {
        return m_position;
    }
};