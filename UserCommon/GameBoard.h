#pragma once
#include "common/BattleInfo.h"
#include "UserCommon/Position.h"
#include "common/TankAlgorithm.h"
#include "common/ActionRequest.h"
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <fstream>
#include <optional>
#include <utility> // for std::pair
#include <cstddef> // for size_t
#include <memory>  // for unique_ptr
#include <tuple>

namespace UserCommon
{
    class GameBoard : public BattleInfo
    {
    private:
        bool is_valid;
        size_t height, width, maxSteps, numShells;
        set<Position> walls;
        set<Position> mines;
        map<Position, int> weakenedWalls;
        vector<tuple<int, int, Position>> tanks;
        vector<pair<Position, string>> shells;
        string name;

    public:
        GameBoard();
        GameBoard(size_t width, size_t height, size_t maxSteps,
                  set<Position> &walls, set<Position> &mines,
                  vector<tuple<int, int, Position>> &&tanks);

        GameBoard(const string &filename);

        // Destructor
        ~GameBoard() override;

        bool isValid() { return is_valid; }
        set<Position> &getWalls() { return walls; }
        set<Position> &getMines() { return mines; }
        map<Position, int> &getWeakenedWalls() { return weakenedWalls; }
        vector<tuple<int, int, Position>> &getTanks() { return tanks; };
        vector<pair<Position, string>> &getShells() { return shells; }

        // Const getters (to be used when only reading the board state)
        const std::set<Position> &getWalls() const { return walls; }
        const std::set<Position> &getMines() const { return mines; }
        const std::map<Position, int> &getWeakenedWalls() const { return weakenedWalls; }
        const std::vector<std::tuple<int, int, Position>> &getTanks() const { return tanks; }
        const std::vector<std::pair<Position, std::string>> &getShells() const { return shells; }
        size_t getHeight() const { return height; }
        size_t getWidth() const { return width; }

        size_t getMaxSteps() { return maxSteps; }
        size_t getHeight() { return height; }
        size_t getWidth() { return width; }
        size_t getNumShells() { return numShells; }

        void addShell(Position pos, string dir) { shells.emplace_back(pos, dir); }
    };
}