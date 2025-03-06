// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "physics/Vec2.hpp"

#include <string>

enum ActionType
{
    START = 0,
    END = 1
};

class Action
{
    std::string m_name;
    ActionType m_type;

public:

    Action() = default;

    Action(const std::string& name, const ActionType type)
        : m_name(name), m_type(type) {
    }

    const std::string& name() const
    {
        return m_name;
    }

    const ActionType type() const
    {
        return m_type;
    }
};