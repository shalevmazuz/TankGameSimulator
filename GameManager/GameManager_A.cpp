#include "GameManager_A.h"
#include <iostream>
#include <vector>
#include "TankState.h"
#include "UserCommon/Directions.h"
#include "UserCommon/SatelliteViewImpl.h"
#include "common/GameManagerRegistration.h"
#include <sstream>

namespace GameManager
{
    using namespace UserCommon;

    GameManager_A::GameManager_A(bool verbose)
        : stepCount(0),
          stepsSinceAmmoEnd(0),
          maxSteps(0),
          board(board_),
          verbose(verbose)
    {
    }

    // true if there is no wall in Position
    bool GameManager_A::isFree(const Position &pos) const
    {
        if (board.getWalls().find(pos) != board.getWalls().end())
        {
            return false;
        }
        return true;
    }

    std::string ActionToString(ActionRequest action)
    {
        switch (action)
        {
        case ActionRequest::MoveForward:
            return "MoveForward";
        case ActionRequest::MoveBackward:
            return "MoveBackward";
        case ActionRequest::RotateLeft90:
            return "RotateLeft90";
        case ActionRequest::RotateRight90:
            return "RotateRight90";
        case ActionRequest::RotateLeft45:
            return "RotateLeft45";
        case ActionRequest::RotateRight45:
            return "RotateRight45";
        case ActionRequest::Shoot:
            return "Shoot";
        case ActionRequest::GetBattleInfo:
            return "GetBattleInfo";
        case ActionRequest::DoNothing:
            return "DoNothing";
        default:
            return "UnknownAction";
        }
    }

    // perform actions on tanks, enforce rules, print actions performed to log file.
    void GameManager_A::applyActionToTank(TankState &tank, const ActionRequest &action, Player &p)
    {
        if (action == ActionRequest::MoveForward)
        {
            tank.setLastAction(ActionRequest::MoveForward);
            if (tank.isPendingBackward())
            {
                tank.setPendingBackward(false);
                tank.setBackwardWait(0);
                // tank.setActionIgnored(true);
            }
            else if (isFree(tank.getPosition() + Directions::directions().at(tank.getDirection())))
            {
                tank.setPosition(tank.getPosition() + Directions::directions().at(tank.getDirection()));
            }
            else
            {
                tank.setActionIgnored(true);
            }
        }
        if (tank.isPendingBackward() && tank.getBackwardWait() == 2)
        {
            tank.setLastAction(action);
            if (isFree(tank.getPosition() + Directions::oppDirections().at(tank.getDirection())))
            {
                tank.setPosition(tank.getPosition() + Directions::oppDirections().at(tank.getDirection()));
                tank.setPendingBackward(false);
                if (action != ActionRequest::MoveBackward)
                {
                    tank.setActionIgnored(true);
                }
            }
            else
            {
                tank.setActionIgnored(true);
            }
        }
        else if (action == ActionRequest::MoveBackward)
        {
            tank.setLastAction(ActionRequest::MoveBackward);
            if (!tank.isPendingBackward() && tank.getBackwardWait() >= 2)
            {
                if (isFree(tank.getPosition() + Directions::oppDirections().at(tank.getDirection())))
                {
                    tank.setPosition(tank.getPosition() + Directions::oppDirections().at(tank.getDirection()));
                }
                else
                {
                    tank.setActionIgnored(true);
                }
            }
            else
            {
                if (tank.getBackwardWait() != 0)
                {
                    tank.setActionIgnored(true);
                }
                tank.setPendingBackward(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
        }
        else if (action == ActionRequest::RotateLeft45)
        {
            tank.setLastAction(ActionRequest::RotateLeft45);
            if (tank.isPendingBackward())
            {
                tank.setActionIgnored(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
            else
            {
                int index = Directions::dirToIndex().at(tank.getDirection());
                tank.setDirection(Directions::directionOrder().at((index - 1 + 8) % 8));
                tank.setPendingBackward(false);
                tank.setBackwardWait(0);
            }
        }
        else if (action == ActionRequest::RotateRight45)
        {
            tank.setLastAction(ActionRequest::RotateRight45);
            if (tank.isPendingBackward())
            {
                tank.setActionIgnored(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
            else
            {
                int index = Directions::dirToIndex().at(tank.getDirection());
                tank.setDirection(Directions::directionOrder().at((index + 1 + 8) % 8));
                tank.setPendingBackward(false);
                tank.setBackwardWait(0);
            }
        }
        else if (action == ActionRequest::RotateLeft90)
        {
            tank.setLastAction(ActionRequest::RotateLeft90);
            if (tank.isPendingBackward())
            {
                tank.setActionIgnored(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
            else
            {
                int index = Directions::dirToIndex().at(tank.getDirection());
                tank.setDirection(Directions::directionOrder().at((index - 2 + 8) % 8));
                tank.setPendingBackward(false);
                tank.setBackwardWait(0);
            }
        }
        else if (action == ActionRequest::RotateRight90)
        {
            tank.setLastAction(ActionRequest::RotateRight90);
            if (tank.isPendingBackward())
            {
                tank.setActionIgnored(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
            else
            {
                int index = Directions::dirToIndex().at(tank.getDirection());
                tank.setDirection(Directions::directionOrder().at((index + 2 + 8) % 8));
                tank.setPendingBackward(false);
                tank.setBackwardWait(0);
            }
        }
        else if (action == ActionRequest::Shoot)
        {
            tank.setLastAction(ActionRequest::Shoot);
            if (tank.isPendingBackward())
            {
                tank.setActionIgnored(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
            else if (tank.getCooldown() == 0 && tank.getAmmo() > 0)
            {
                board.addShell(tank.getPosition(), tank.getDirection());
                tank.setCooldown(4);
                tank.setAmmo(tank.getAmmo() - 1);
                tank.setPendingBackward(false);
                tank.setBackwardWait(0);
            }
            else
            {
                tank.setActionIgnored(true);
            }
        }
        else if (action == ActionRequest::DoNothing)
        {
            tank.setLastAction(ActionRequest::DoNothing);
            if (tank.isPendingBackward())
            {
                tank.setActionIgnored(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
        }
        else if (action == ActionRequest::GetBattleInfo)
        {
            tank.setLastAction(ActionRequest::GetBattleInfo);
            if (tank.isPendingBackward())
            {
                tank.setActionIgnored(true);
                tank.setBackwardWait(tank.getBackwardWait() + 1);
            }
            else
            {
                auto view = make_unique<SatelliteViewImpl>(board, tank.getPosition());
                auto it = tankAlgorithms.find({tank.getPlayerIdx(), tank.getTankIdx()});
                if (it != tankAlgorithms.end())
                {
                    TankAlgorithm *algoPtr = it->second.get(); // raw pointer if needed
                    p.updateTankWithBattleInfo(*algoPtr, *view);
                }
            }
        }
    }

    // update the game board according to tanks actions
    void GameManager_A::applyActions(
        std::map<std::pair<int, int>, ActionRequest> &actions,
        Player &p1, Player &p2)
    {
        for (auto &tank : tankStates)
        {
            if (!tank->isAlive())
                continue;
            auto key = std::make_pair(tank->getPlayerIdx(), tank->getTankIdx());
            if (actions.find(key) != actions.end())
            {
                if (tank->getPlayerIdx() == 1)
                    applyActionToTank(*tank, actions[key], p1);
                else
                    applyActionToTank(*tank, actions[key], p2);
            }
        }
    }

    // handles cases when shell hit wall
    void GameManager_A::shellHitWall()
    {
        std::vector<std::pair<Position, std::string>> survivors;
        survivors.reserve(board.getShells().size());

        for (auto &shell : board.getShells())
        {
            if (board.getWalls().count(shell.first))
            {
                board.getWeakenedWalls()[shell.first]++;
                if (board.getWeakenedWalls()[shell.first] == 2)
                {
                    board.getWalls().erase(shell.first);
                }
            }
            else
            {
                survivors.push_back(shell);
            }
        }

        board.getShells() = std::move(survivors);
    }

    // handles cases of tanks collision
    void GameManager_A::tankHitTank()
    {
        map<Position, int> posCount;

        // Count positions of only alive tanks
        for (auto &tank : tankStates)
        {
            if (tank->isAlive())
            {
                posCount[tank->getPosition()]++;
            }
        }

        // Kill tanks that are alive and share a position with another alive tank
        for (auto &tank : tankStates)
        {
            if (tank->isAlive() && posCount[tank->getPosition()] > 1)
            {
                tank->setIsAlive(false);
                tank->setWasKilledThisRound(true);
            }
        }
    }

    // handles cases of shell hitting a tank
    void GameManager_A::shellHitTank()
    {
        std::vector<std::pair<Position, std::string>> survivors;
        survivors.reserve(board.getShells().size());

        for (auto &shell : board.getShells())
        {
            bool hitTank = false;
            for (auto &tank : tankStates)
            {
                if (tank->isAlive() && shell.first == tank->getPosition())
                {
                    tank->setIsAlive(false);
                    tank->setWasKilledThisRound(true);
                    hitTank = true;
                    break;
                }
            }
            if (!hitTank)
                survivors.push_back(shell);
        }

        board.getShells() = std::move(survivors);
    }

    // handles cases when tanks step on a mine
    void GameManager_A::tankHitMine()
    {
        for (auto &tank : tankStates)
        {
            if (board.getMines().find(tank->getPosition()) != board.getMines().end())
            {
                board.getMines().erase(tank->getPosition());
                tank->setIsAlive(false);
                tank->setWasKilledThisRound(true);
            }
        }
    }

    // handles cases when shells colided. Generated by ChatGPT.
    void GameManager_A::shellHitShell(const std::vector<std::pair<Position, string>> &prevShells)
    {
        map<Position, int> posCount;
        std::set<size_t> toRemove;

        // Count how many times each new position appears
        const auto &shells = board.getShells();
        for (const auto &shell : shells)
        {
            posCount[shell.first]++;
        }

        // Mark all shells that collide in the same position
        for (size_t i = 0; i < shells.size(); ++i)
        {
            if (posCount[shells[i].first] > 1)
            {
                toRemove.insert(i);
            }
        }

        // Detect and mark crossing shells (swap positions)
        for (size_t i = 0; i < shells.size(); ++i)
        {
            for (size_t j = i + 1; j < shells.size(); ++j)
            {
                if (prevShells[i].first == shells[j].first &&
                    prevShells[j].first == shells[i].first)
                {
                    toRemove.insert(i);
                    toRemove.insert(j);
                }
            }
        }

        // Remove all marked shells
        std::vector<std::pair<Position, string>> filtered;
        for (size_t i = 0; i < shells.size(); ++i)
        {
            if (toRemove.find(i) == toRemove.end())
            {
                filtered.push_back(shells[i]);
            }
        }

        board.getShells() = std::move(filtered);
    }

    // update the board according to all movment accross the board. Since shells are twice as fast as tanks, the tanks moves only on even steps.
    void GameManager_A::advanceStep(Player &p1, Player &p2)
    {
        std::map<std::pair<int, int>, ActionRequest> actions;

        if (stepCount % 2 == 0) // even steps → tanks act
        {
            bool isAmmoEnd = true;

            for (auto &tank : tankStates)
            {
                if (!tank->isAlive())
                    continue;

                // reduce cooldown if needed
                if (tank->getCooldown() > 0)
                    tank->setCooldown(tank->getCooldown() - 1);

                if (tank->getAmmo() > 0)
                    isAmmoEnd = false;

                auto it = tankAlgorithms.find({tank->getPlayerIdx(), tank->getTankIdx()});
                if (it != tankAlgorithms.end())
                {
                    TankAlgorithm *algoPtr = it->second.get();
                    actions[{tank->getPlayerIdx(), tank->getTankIdx()}] = algoPtr->getAction();
                }
            }

            if (isAmmoEnd)
                stepsSinceAmmoEnd++;

            // apply actions using the fixed map
            applyActions(actions, p1, p2);

            // move shells
            std::vector<std::pair<Position, std::string>> prevShells = board.getShells();
            for (size_t i = 0; i < board.getShells().size(); i++)
            {
                board.getShells()[i].first =
                    board.getShells()[i].first +
                    Directions::directions().at(board.getShells()[i].second);
            }

            // resolve collisions
            shellHitWall();
            tankHitTank();
            tankHitMine();
            shellHitTank();
            shellHitShell(prevShells);
        }
        else // odd steps → only shells move
        {
            std::vector<std::pair<Position, std::string>> prevShells = board.getShells();
            for (size_t i = 0; i < board.getShells().size(); i++)
            {
                board.getShells()[i].first =
                    board.getShells()[i].first +
                    Directions::directions().at(board.getShells()[i].second);
            }

            shellHitWall();
            shellHitTank();
            shellHitShell(prevShells);
        }
    }

    // true if one of the tanks destroyed or 40 steps passed since ammo ends or the game get to step limit
    bool GameManager_A::isGameOver() const
    {
        if (stepsSinceAmmoEnd >= STEPSAFTERAMMOENDS || stepCount >= maxSteps * 2)
        {
            return true;
        }
        int player1Counter = 0, player2Counter = 0;
        for (auto &tank : tankStates)
        {
            if (tank->isAlive())
            {
                if (tank->getPlayerIdx() == 1)
                {
                    ++player1Counter;
                }
                if (tank->getPlayerIdx() == 2)
                {
                    ++player2Counter;
                }
            }
        }
        if (player1Counter == 0 || player2Counter == 0)
        {
            return true;
        }
        return false;
    }

    std::string join(const std::vector<std::string> &items, const std::string &delimiter)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (i > 0)
                oss << delimiter;
            oss << items[i];
        }
        return oss.str();
    }

    GameResult GameManager_A::run(
        size_t map_width, size_t map_height,
        const SatelliteView &map,
        std::string map_name,
        size_t max_steps, size_t num_shells,
        Player &player1, std::string name1,
        Player &player2, std::string name2,
        TankAlgorithmFactory player1_tank_algo_factory,
        TankAlgorithmFactory player2_tank_algo_factory)
    {
        maxSteps = max_steps;
        if (verbose)
        {
            // Construct the file name using map name and player names
            std::stringstream file_name;
            file_name << "./game_output_" << "_" << name1 << "_vs_" << name2 << "_" << map_name << ".txt";

            // Open the output file
            output_file.open(file_name.str(), ios::out | ios::trunc);
            if (!output_file.is_open())
            {
                throw runtime_error("Failed to open output file: " + file_name.str());
            }
        }
        set<Position> walls;
        set<Position> mines;
        vector<tuple<int, int, Position>> tanks;
        vector<Position> shells;
        int p1tanks = 0;
        int p2tanks = 0;

        for (size_t i = 0; i < map_width; i++)
        {
            for (size_t j = 0; j < map_height; j++)
            {
                char obj = map.getObjectAt(i, j);

                if (obj == '#')
                {
                    walls.insert(Position(i, j));
                }
                else if (obj == '*')
                {
                    shells.emplace_back(Position(i, j));
                }
                else if (obj == '@')
                {
                    mines.insert(Position(i, j));
                }
                else if (obj == '1' || obj == '2')
                {
                    if (obj == '1')
                    {
                        ++p1tanks;
                        tanks.emplace_back(1, p1tanks, Position(i, j));
                        tankStates.emplace_back(make_unique<TankState>(1, p1tanks, num_shells, Position(i, j)));
                        tankAlgorithms[{1, p1tanks}] = player1_tank_algo_factory(1, p1tanks);
                    }
                    else
                    {
                        ++p2tanks;
                        tanks.emplace_back(2, p2tanks, Position(i, j));
                        tankStates.emplace_back(make_unique<TankState>(2, p2tanks, num_shells, Position(i, j)));
                        tankAlgorithms[{2, p2tanks}] = player2_tank_algo_factory(2, p2tanks);
                    }
                }
            }
        }
        GameBoard board_(map_width, map_height, max_steps, walls, mines, move(tanks));
        board = move(board_);

        while (!isGameOver())
        {

            vector<string> roundLog;
            advanceStep(player1, player2);

            if (stepCount % 2 == 0)
            {
                for (auto &tank : tankStates)
                {
                    if (!tank->isAlive() && !tank->getWasKilledThisRound())
                    {
                        roundLog.push_back("killed");
                        continue;
                    }

                    std::string actionStr = ActionToString(tank->getLastAction());

                    if (tank->isActionIgnored())
                        actionStr += " (ignored)";

                    if (tank->getWasKilledThisRound())
                        actionStr += " (killed)";
                    roundLog.push_back(actionStr);
                }

                output_file << join(roundLog, ", ") << "\n";
            }
            stepCount++;
            // Reset per-round flags
            for (auto &tank : tankStates)
            {
                tank->setWasKilledThisRound(false);
                tank->setActionIgnored(false);
            }
        }

        GameResult result;
        result.remaining_tanks.resize(2, 0);

        for (auto &tank : tankStates)
        {
            if (tank->isAlive())
            {
                int idx = tank->getPlayerIdx() - 1;
                result.remaining_tanks[idx]++;
            }
        }

        if (result.remaining_tanks[0] > 0 && result.remaining_tanks[1] == 0)
        {
            result.winner = 1;
            result.reason = GameResult::ALL_TANKS_DEAD;
            output_file << "Player 1 won with " << result.remaining_tanks[0] << " tanks still alive" << "\n";
        }
        else if (result.remaining_tanks[1] > 0 && result.remaining_tanks[0] == 0)
        {
            result.winner = 2;
            result.reason = GameResult::ALL_TANKS_DEAD;
            output_file << "Player 2 won with " << result.remaining_tanks[1] << " tanks still alive" << "\n";
        }
        else if (result.remaining_tanks[0] == 0 && result.remaining_tanks[1] == 0)
        {
            result.winner = 0;
            result.reason = GameResult::ALL_TANKS_DEAD;
            output_file << "Tie, both players have zero tanks" << "\n";
        }
        else if (stepsSinceAmmoEnd >= STEPSAFTERAMMOENDS)
        {
            result.winner = 0;
            result.reason = GameResult::ZERO_SHELLS;
            output_file << "Tie, both players have zero shells for " << STEPSAFTERAMMOENDS << " steps" << "\n";
        }
        else
        {
            result.winner = 0;
            result.reason = GameResult::MAX_STEPS;
            output_file << "Tie, reached max steps = " << maxSteps << ", player 1 has " << result.remaining_tanks[0] << " tanks, player 2 has " << result.remaining_tanks[1] << " tanks" << "\n";
        }

        auto &tanks1 = board.getTanks();
        tanks1.erase(
            std::remove_if(tanks1.begin(), tanks1.end(),
                           [&](const auto &tankTuple)
                           {
                               const auto &[owner, id, pos] = tankTuple;
                               for (auto &tank : tankStates)
                               {
                                   if (!tank->isAlive() && id == tank->getTankIdx() && owner == tank->getPlayerIdx())
                                       return true; // remove dead tank
                               }
                               return false;
                           }),
            tanks1.end());
        result.gameState = make_unique<SatelliteViewImpl>(board, Position());
        result.rounds = stepCount / 2;

        this->maxSteps = 0;
        this->stepCount = 0;
        this->stepsSinceAmmoEnd = 0;
        board = GameBoard();
        tankAlgorithms.clear();
        tankStates.clear();
        output_file.close();

        return result;
    }

    std::unique_ptr<AbstractGameManager> createGameManager(bool verbose)
    {
        return std::make_unique<GameManager_A>(verbose);
    }
}
using GameManager::GameManager_A;
REGISTER_GAME_MANAGER(GameManager_A)