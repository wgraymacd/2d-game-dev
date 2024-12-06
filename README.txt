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
- Animation N T F S | name, texture/sprite name, frame count, speed | std::string (x2), int (x2)
- AnimationStatic N T X Y W H | name texture name, pixelX, pixelY, width, height | std::string (x2), int (x4)
- Font N P | name, file path | std::string (x2)

level config:
notes: grid coord systems (0, 0) is in the bottom left to make things more intuitive, all (GX, GY) positions are in grid coords, cells 64x64 pixels
entity should be positioned such that bottom left corner is aligned with bottom left corner of grid coordinate (not all textures are the size 64x64 pixels)
- Tile N GX GY | animation name, grid x, grid y | std::string, float (x2)
- Dec N GX GY | anim name, grid x, grid y | std::string, float (x2)
- Player GX GY CW CH SX SY SM GRAVITY BA | grid pos, bounding box width/height, horizontal speed, jump speed, max speed, gravity, bullet animation | float, ..., float, std::string





TOP DOWN GAME A3 ZELDA:

Project Info:
-------------

assets:

entities rendered with textures and animations
will also use fonts and sounds
loaded once at beginning and stored in Assets class, which is stored by the GameEngine class
all Assets defined in assets.txt, with syntax defined below

player:
- your typical Top-Down 2D platformer controls of up, down, left, right, attack (for all directions), this may evolve as I toon this to my liking

other entities:
- either block movement or vision now, not just movement (line of sight functionality)

NPCs:
- given AI behavior (either follow or patrol)
- take note of oscillating when NPCs returning to home positioned

animations:
- implemented by storing multiple frames inside a texture
- can be repeating (loop forever) or non-repeating
- if non-repeating, should be destroyed once animation has ended (one cycle)
- will use everywhere, even if just one frame (not using textures explicitly in code)
- will also use for textures in a texture atlas (different spec in config files)

decoration:
- drawn to the screen, do not interact with other entities
- can be given any animation

tiles:
- entities that define the level geometry and interact with players
- given animation defined in assets file
- tiles given CBoundingBox equal to size of the animation

drawing:
- change camera position (window.view()) of the game window to alternate between two modes based on the m_follow variable
- true: follow mode - camera centered on player
- false: room mode - camera centered on middle of room

rooms:
- each level split up into rooms (rx, ry)
- defined by rect (rx * w, ry * h, w, h)
- each tile given a (tx, ty) grid pos in room
- camera centered on room that player is in

misc:
- 'p' pauses game
- 't' toggles drawing textures
- 'c' toggles drawing of bounding boxes
- 'g' toggles drawing of grid
- 'y' toggles camera mode
- 'esc' goes back to main menu or quits if on main menu

config files:
-------------

asset config:
- Texture N P | name, file path | std::string (x2)
- Animation N T F S | name, texture/sprite name, frame count, speed | std::string (x2), int (x2)
- Font N P | name, file path | std::string (x2)
- Sound N P | name, path | std::string (x2)

level config:
- Player X Y BX BY S H | spawn pos, bounding box size, spped, health | int (x4), float, int
- Tile N RX RY TX TY BM BV | anim name, room coord, tile pos, block movement, blocks vision | std::string, int (x6)
- NPC N RX RY TX TY BM BV H D AI ... | anim name, room coord, tile pos, block move, blocks vision, max health, damage, AI behavior, AI params | std::string, int (x8), std::string, see below
    - AI = follow: ... = S | follow speed | float
    - AI = patrol: ... = S N X1 Y1 X2 Y2 ... XN YN | patrol speed, patrol positions, position 1 to N | float, int, int (x2N)