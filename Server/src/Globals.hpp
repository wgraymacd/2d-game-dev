#pragma once

using NetEntityID = int; // ensure this is consistent with client side EntityID

namespace GlobalSettings
{
    inline const NetEntityID worldMaxEntities = 1000; // not including tiles or other things outside of main entity memory pool
}
