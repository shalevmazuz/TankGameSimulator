#pragma once
#include "Position.h"
#include <map>
#include <string>
#include <vector>

namespace UserCommon
{
    class Directions
    {
    public:
        static const std::map<std::string, Position> &directions();
        static const std::map<std::string, Position> &oppDirections();
        static const std::vector<std::string> &directionOrder();
        static const std::map<std::string, int> &dirToIndex();
    };
}
