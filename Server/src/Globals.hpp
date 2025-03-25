// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

using EntityID = int; // ensure this is consistent with client side EntityID

namespace Settings {
    inline constexpr EntityID worldMaxEntities = 1000; // not including tiles or other things outside of main entity memory pool
    inline constexpr int worldMaxCellsX = 4000;
    inline constexpr int worldMaxCellsY = 1000;
    inline constexpr int maxLobbyPlayers = 50;
}
