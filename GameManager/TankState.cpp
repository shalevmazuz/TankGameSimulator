#include "TankState.h"

namespace GameManager
{
    using namespace UserCommon;
    TankState::TankState(int player_idx, int tank_idx, int ammo, Position pos)
        : pos(pos),
          player_idx(player_idx),
          tank_idx(tank_idx),
          is_Alive(true),
          pendingBackward(false),
          ammo(ammo),
          backwardWait(0),
          cooldown(0)
    {
        direction = (player_idx == 1) ? "L" : "R";
    }
    TankState::~TankState() = default;
}