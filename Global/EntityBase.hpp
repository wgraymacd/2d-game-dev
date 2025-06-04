#pragma once

using EntityID = unsigned int;

class EntityBase
{
public:
    enum Type
    {
        PLAYER,
        ENEMY,
        WEAPON,
        BODY_PART,
        BULLET,
        RAGDOLL_PART,
        NUM_TYPES
    };
};
