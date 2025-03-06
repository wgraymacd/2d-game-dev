// Copyright 2025, William MacDonald, All Rights Reserved.

#include "core/GameEngine.hpp"

int main(int argc, char* argv[])
{
    GameEngine g("../bin/assets.txt");
    g.run();

    return EXIT_SUCCESS;
}