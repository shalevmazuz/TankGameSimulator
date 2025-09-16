#pragma once
#include "common/Player.h"
#include "common/PlayerRegistration.h"
#include "UserCommon/GameBoard.h"

namespace Algorithm
{

    class MyPlayer : public Player
    {
    private:
        int index;
        size_t x, y, max_steps, num_shells;

    public:
        MyPlayer(int Myplayer_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
        ~MyPlayer() = default;
        void updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satellite_view) override;
    };
}
