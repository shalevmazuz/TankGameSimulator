#pragma once
#include "common/AbstractGameManager.h"
#include "common/GameResult.h"
#include "common/Player.h"
#include "common/TankAlgorithm.h"
#include "TankState.h"
#include "UserCommon/GameBoard.h"
#include "common/GameManagerRegistration.h"
#include <memory>
#include <vector>
#include <string>

namespace GameManager
{
    using namespace std;

    class GameManager_A : public AbstractGameManager
    {
    public:
        GameManager_A(bool verbose);
        ~GameManager_A() {}

        GameResult run(
            size_t map_width, size_t map_height,
            const SatelliteView &map, // <= a snapshot, NOT updated
            string map_name,
            size_t max_steps, size_t num_shells,
            Player &player1, string name1, Player &player2, string name2,
            TankAlgorithmFactory player1_tank_algo_factory,
            TankAlgorithmFactory player2_tank_algo_factory) override;

    private:
        void applyActionToTank(TankState &tank, const ActionRequest &action, Player &p);
        void applyActions(
            std::map<std::pair<int, int>, ActionRequest> &actions,
            Player &p1, Player &p2);
        void shellHitWall();
        void tankHitTank();
        void shellHitTank();
        void tankHitMine();
        void shellHitShell(const std::vector<std::pair<UserCommon::Position, string>> &prevShells);
        void advanceStep(Player &p1, Player &p2);
        bool isGameOver() const;
        void printGameResult() const;
        bool isFree(const UserCommon::Position &pos) const;

        size_t stepCount, stepsSinceAmmoEnd, maxSteps;
        UserCommon::GameBoard &board;
        bool verbose;
        ofstream output_file;
        UserCommon::GameBoard board_ = UserCommon::GameBoard();

        std::map<std::pair<int, int>, unique_ptr<TankAlgorithm>> tankAlgorithms;
        std::vector<unique_ptr<TankState>> tankStates;
        size_t STEPSAFTERAMMOENDS = 40;
    };

}