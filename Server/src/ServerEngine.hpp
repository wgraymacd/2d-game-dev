// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "NetworkManager.hpp"

#include <string>
#include <chrono>

class ServerEngine
{
    NetworkManager m_netManager;
    bool m_running = true;

    void update();

public:
    ServerEngine() = default;

    void run();
};