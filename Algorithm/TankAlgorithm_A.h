#pragma once
#include "common/TankAlgorithm.h"
#include "common/TankAlgorithmRegistration.h"
#include "UserCommon/Position.h"
#include "UserCommon/GameBoard.h"
#include <vector>
#include <set>
#include <map>
#include <string>
#include <queue>
#include <utility>
#include <cstddef>

namespace Algorithm
{

    class TankAlgorithm_A : public TankAlgorithm
    {
    private:
        std::queue<ActionRequest> path;
        std::vector<std::tuple<int, int, UserCommon::Position>> tanks;
        std::string direction;
        int playerIndex, tankIndex;
        UserCommon::Position pos;
        int turn_num;
        UserCommon::GameBoard board;

        std::set<UserCommon::Position> computeDangerZones();
        bool isShootPossible();
        bool isFree(const UserCommon::Position &pos);
        int manhattan(const UserCommon::Position &a, const UserCommon::Position &b);
        UserCommon::Position findClosestEnemyTank();
        std::queue<ActionRequest> getActionsToEnemyTank(UserCommon::Position pos_other);

    public:
        TankAlgorithm_A(int player_index, int tank_index);
        ~TankAlgorithm_A();
        ActionRequest getAction() override;
        void updateBattleInfo(BattleInfo &info) override;
    };

}