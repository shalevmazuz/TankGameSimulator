#include <iostream>
#include "Position.h"
#include <string>
#include <set>
#include <vector>
#include <utility> // for std::pair
#include <cstddef> // for size_t
#include <tuple>

namespace UserCommon
{
    using namespace std;

    int Position::width = 0;
    int Position::height = 0;
    Position::Position() : x(-1), y(-1) {};
    Position::Position(int x, int y) : x(x), y(y) {};

    bool Position::operator<(const Position &other) const
    {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }

    bool Position::operator==(const Position &other) const
    {
        return x == other.x && y == other.y;
    }

    bool Position::operator!=(const Position &other) const
    {
        return x != other.x || y != other.y;
    }

    Position Position::operator+(const Position &other) const
    {
        int newX = ((x + other.x) % width + width) % width;
        int newY = ((y + other.y) % height + height) % height;
        return Position(newX, newY);
    }

}