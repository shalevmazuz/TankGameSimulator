#include "Directions.h"

namespace UserCommon
{

    const std::map<std::string, Position> &Directions::directions()
    {
        static const auto *m = new std::map<std::string, Position>{
            {"U", Position(0, -1)}, {"UR", Position(1, -1)}, {"R", Position(1, 0)}, {"DR", Position(1, 1)}, {"D", Position(0, 1)}, {"DL", Position(-1, 1)}, {"L", Position(-1, 0)}, {"UL", Position(-1, -1)}};
        return *m;
    }

    const std::map<std::string, Position> &Directions::oppDirections()
    {
        static const auto *m = new std::map<std::string, Position>{
            {"U", Position(0, 1)}, {"UR", Position(-1, 1)}, {"R", Position(-1, 0)}, {"DR", Position(-1, -1)}, {"D", Position(0, -1)}, {"DL", Position(1, -1)}, {"L", Position(1, 0)}, {"UL", Position(1, 1)}};
        return *m;
    }

    const std::vector<std::string> &Directions::directionOrder()
    {
        static const auto *v = new std::vector<std::string>{
            "U", "UR", "R", "DR", "D", "DL", "L", "UL"};
        return *v;
    }

    const std::map<std::string, int> &Directions::dirToIndex()
    {
        static const auto *m = new std::map<std::string, int>{
            {"U", 0}, {"UR", 1}, {"R", 2}, {"DR", 3}, {"D", 4}, {"DL", 5}, {"L", 6}, {"UL", 7}};
        return *m;
    }
}
