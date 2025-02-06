# Current State of the Game


# Big-Picture Goal:
I am building a 2D video game from scratch using:
- ECS architecture
- C++ and only the SFML library (will attempt using lower-level libraries later on)

I will build a 3D game engine (or simulator, something like that) using this game as a template, focusing mainly on the physics, math, and logic. I aim to practice 3D math and physics for use in games, simulators, AR/VR, etc.

## This Repo:
This repo will become my 2D game. It will go private eventually when it starts to grow more and I decide to start selling it.

### config files:
asset config:
- Texture N P | name, file path | std::string (x2)
- Animation N T F S | name, texture/sprite name, frame count, speed | std::string (x2), int (x2)
- AnimationStatic N T X Y W H | name texture name, pixelX, pixelY, width, height | std::string (x2), int (x4)
- Font N P | name, file path | std::string (x2)

#### misc:
'p' pauses game
't' toggles drawing textures
'c' toggles drawing of bounding boxes
'g' toggles drawing of grid
'esc' goes back to main menu or quits if on main menu