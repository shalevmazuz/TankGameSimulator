#pragma once
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <utility>
#include <cstddef>
#include <tuple>

namespace UserCommon
{
    using namespace std;

    class Position
    {
    public:
        static int width, height;
        int x, y;
        Position();
        Position(int x, int y);
        bool operator<(const Position &other) const;
        bool operator==(const Position &other) const;
        Position operator+(const Position &other) const;
        bool operator!=(const Position &other) const;
    };
}