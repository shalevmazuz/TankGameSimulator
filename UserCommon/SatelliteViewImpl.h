#pragma once
#include "common/SatelliteView.h"
#include "GameBoard.h"
#include "UserCommon/Position.h"

namespace UserCommon
{

    class SatelliteViewImpl : public SatelliteView
    {
    private:
        size_t height, width;
        std::set<Position> walls;
        std::set<Position> mines;
        std::map<Position, int> weakenedWalls;
        std::vector<std::tuple<int, int, Position>> tanks;
        std::vector<std::pair<Position, std::string>> shells;
        Position tankPos;

    public:
        SatelliteViewImpl(const GameBoard &board, Position tankPos);
        char getObjectAt(size_t x, size_t y) const override;
    };
}