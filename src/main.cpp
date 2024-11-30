#include <SFML/Graphics.hpp>
#include "GameEngine.h"

int main()
{
    GameEngine g("../bin/assets.txt");
    g.run();
}