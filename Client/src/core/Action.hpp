// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// C++ standard libraries
#include <string>

enum ActionType
{
    START,
    END
};

class Action
{
    std::string m_name;
    ActionType m_type;

public:

    Action() = default;

    Action(const std::string& name, ActionType type)
        : m_name(name), m_type(type)
    { }

    const std::string& name() const
    {
        return m_name;
    }

    ActionType type() const
    {
        return m_type;
    }
};
