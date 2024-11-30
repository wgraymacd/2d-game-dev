Big-Picture Goal:
=================
I am building a 2D video game from scratch using:
- ECS architecture
- C++ and only the SFML library (will attempt using lower-level libraries later on)
Ultimately, I will build a 3D game engine (or simulator, something like that), focusing mainly on the physics, math, and logic. I aim to master 3D math for use in games, simulators, AR/VR, etc.

This Repo:
==========
This repo is a recreation of Mario Bros, a small project I took on to learn some things before creating my own game.

Acknowledgements:
-----------------
I referrenced some of the skeleton code for this project from a course assignment written by Dave Churchill (https://www.youtube.com/@DaveChurchill) in Fall 2024.

Project Info:
-------------

note: all entity positions denote the center of their rectangular sprite and bounding box (if they have one) set vie sprite.setOrigin() in the Animation class constructor

assets:

entities rendered with textures and animations
loaded once at beginning and stored in Assets class, which is stored by the GameEngine class
all Assets defined in assets.txt, with syntax defined below

player:
- your typical 2D platformer controls
- jump key held: higher jump, not continuous jumping
- can't move through tile entities
- can move through decoration entities
- respawn at start if fall below screen
- affected by downward gravity
- has max speed specified in level file 

animations:
- implemented by storing multiple frames inside a texture
- Animation class handles frame advancement based on animation speed 
- can be repeating (loop forever) or non-repeating
- if non-repeating, should be destroyed once animation has ended (one cycle)
- will use everywhere, even if just one frame (not using textures explicitly in code)

decoration:
- drawn to the screen, do not interact with other entities
- can be given any animation

tiles:
- entities that define the level geometry and interact with players
- given animation defined in assets file
- tiles given CBoundingBox equal to size of the animation
- different behavior depending on which animation they are given

    brick tiles:
    - brick animation 
    - when a brick tile collides with bullet or hit by player from below:
        - change animation to explosion
        - remove bounding box component

    question tiles
    - question tiles given the question animation when created
    - when hit from below by player:
        - animation changes to darker question animation
        - entity with temp lifespan and coin animation should appear above the question tile for 30 frames (64 pixels above since all tiles are 64 pixels)

GUI:

could be useful to have buttons to turn off systems individually, like collisions, animations, etc.
list ID, tag, etc. for each entity in the game, have option to destroy them or remove certain components

rendering:
- entities are rendered in the order that they are stored in the EntityManager, later ones drawn on top of earlier ones

misc:

'p' pauses game
't' toggles drawing textures
'c' toggles drawing of bounding boxes
'g' toggles drawing of grid
'esc' goes back to main menu or quits if on main menu

config files:

asset config:
- Texture N P | name, file path | std::string (x2)
- Animation N T F S | name, texture name, fram count, speed | std::string (x2), int (x2)
- Font N P | name, file path | std::string (x2)

level config:
notes: grid coord systems (0, 0) is in the bottom left to make things more intuitive, all (GX, GY) positions are in grid coords, cells 64x64 pixels
entity should be positioned such that bottom left corner is aligned with bottom left corner of grid coordinate (not all textures are the size 64x64 pixels)
- Tile N GX GY | animation name, grid x, grid y | std::string, float (x2)
- Dec N GX GY | anim name, grid x, grid y | std::string, float (x2)
- Player GX GY CW CH SX SY SM GY B | grid pos, bounding box width/height, horizontal speed, jump speed, max speed, gravity, bullet animation | float, ..., float, std::string

hints:

- aside from what is "already implemented"
- implement Animation::update() and Animation::hasEnded() whenever, independent of gameplay
- implement Scene_Play::loadLevel() first (requires implementing gridToMidPixel first)
- spawnPlayer()
- implement movement so that can start testing the rest
- spawnBullet()
- Physics namespace
