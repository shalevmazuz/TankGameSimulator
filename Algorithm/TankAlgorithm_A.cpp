#include "TankAlgorithm_A.h"
#include "UserCommon/Directions.h"
#include "common/TankAlgorithmRegistration.h"

namespace Algorithm
{
  using namespace UserCommon;
  using namespace std;

  std::string actionToString(ActionRequest action)
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

  map<ActionRequest, int> actionToIndex = {
      {ActionRequest::RotateLeft45, 7}, {ActionRequest::RotateRight45, 1}, {ActionRequest::RotateLeft90, 6}, {ActionRequest::RotateRight90, 2}};

  vector<tuple<int, int, Position>> emptyTanks = {};

  // Constructor
  TankAlgorithm_A::TankAlgorithm_A(int player_index, int tank_index)
      : tanks(emptyTanks), playerIndex(player_index), tankIndex(tank_index), turn_num(-1)
  {
    direction = (player_index == 1) ? "L" : "R";
  }

  // Destructor
  TankAlgorithm_A::~TankAlgorithm_A() = default;

  // Updates game state
  void TankAlgorithm_A::updateBattleInfo(BattleInfo &info)
  {
    GameBoard *gb_ptr = dynamic_cast<GameBoard *>(&info);
    if (gb_ptr)
    {
      board = *gb_ptr;
    }
    else
    {
      std::cerr << "Error: BattleInfo is not a GameBoard\n";
    }
    tanks = board.getTanks();
    for (const auto &[player_idx, tank_idx, tank_pos] : tanks)
    {
      if (tank_idx == -2)
      {
        pos = tank_pos;
      }
    }
  }

  // BFS to find actions to closest enemy tank
  queue<ActionRequest> TankAlgorithm_A::getActionsToEnemyTank(Position pos_other)
  {
    if (pos_other == Position(-1, -1))
    {
      return {};
    }

    using State = tuple<int, Position, string, vector<ActionRequest>>;
    auto cmp = [](const State &a, const State &b)
    {
      return get<0>(a) > get<0>(b);
    };

    priority_queue<State, vector<State>, decltype(cmp)> q(cmp);
    set<pair<Position, string>> visited;

    Position startPos = this->pos;
    string startDir = this->direction;

    q.push({manhattan(startPos, pos_other), startPos, startDir, {}});
    visited.insert({startPos, startDir});

    while (!q.empty())
    {
      auto [dist, pos, dir, actions] = q.top();
      q.pop();

      if (manhattan(pos, pos_other) <= 1)
      {
        queue<ActionRequest> actionQueue;
        for (const auto &act : actions)
          actionQueue.push(act);
        return actionQueue;
      }

      for (const auto &action : {
               ActionRequest::MoveForward,
               ActionRequest::MoveBackward,
               ActionRequest::RotateLeft45,
               ActionRequest::RotateRight45,
               ActionRequest::RotateLeft90,
               ActionRequest::RotateRight90})
      {
        Position newPos = pos;
        string newDir = dir;
        bool valid = true;

        if (action == ActionRequest::MoveForward)
        {
          newPos = pos + Directions::directions().at(dir);
          if (!isFree(newPos))
            valid = false;
        }
        else if (action == ActionRequest::MoveBackward)
        {
          newPos = pos + Directions::oppDirections().at(dir);
          if (!isFree(newPos))
            valid = false;
        }
        else
        {
          int idx = Directions::dirToIndex().at(dir);
          switch (action)
          {
          case ActionRequest::RotateLeft45:
            newDir = Directions::directionOrder()[(idx + 7) % 8];
            break;
          case ActionRequest::RotateRight45:
            newDir = Directions::directionOrder()[(idx + 1) % 8];
            break;
          case ActionRequest::RotateLeft90:
            newDir = Directions::directionOrder()[(idx + 6) % 8];
            break;
          case ActionRequest::RotateRight90:
            newDir = Directions::directionOrder()[(idx + 2) % 8];
            break;
          default:
            break;
          }
        }

        if (valid && visited.find({newPos, newDir}) == visited.end())
        {
          visited.insert({newPos, newDir});
          auto newActions = actions;
          newActions.push_back(action);
          int newDist = manhattan(newPos, pos_other);
          q.push({newDist, newPos, newDir, newActions});
        }
      }
    }

    return queue<ActionRequest>();
  }

  // return next action
  ActionRequest TankAlgorithm_A::getAction()
  {
    turn_num++;

    // Request battle info every 5 turns
    if (turn_num % 3 == 0)
    {
      return ActionRequest::GetBattleInfo;
    }

    // Clear path after requesting info
    if (turn_num % 3 == 1)
    {
      while (!path.empty())
        path.pop();
    }

    // Shoot if aligned and ammo available
    if (isShootPossible())
    {
      return ActionRequest::Shoot;
    }

    // Try to escape danger if currently in a danger zone
    Position currentPos = this->pos;
    string currentDir = this->direction;
    set<Position> dangerZones = computeDangerZones();

    if (dangerZones.count(currentPos))
    {
      Position forwardPos = currentPos + Directions::directions().at(currentDir);
      if (isFree(forwardPos) && dangerZones.count(forwardPos) == 0)
      {
        return ActionRequest::MoveForward;
      }

      // Try rotating to a safe direction then move forward
      for (auto const &[rotation, indexOffset] : actionToIndex)
      {
        int dirIndex = Directions::dirToIndex().at(currentDir);
        string newDir = Directions::directionOrder()[(dirIndex + indexOffset) % 8];
        Position newPos = currentPos + Directions::directions().at(newDir);

        if (isFree(newPos) && dangerZones.count(newPos) == 0)
        {
          path.push(rotation);
          path.push(ActionRequest::MoveForward);
          break;
        }
      }

      // If all else fails, try moving backward
      if (path.empty())
      {
        Position backPos = currentPos + Directions::oppDirections().at(currentDir);
        if (isFree(backPos) && dangerZones.count(backPos) == 0)
        {
          path.push(ActionRequest::MoveBackward);
        }
      }
    }

    // If path is empty, compute a new one to the closest tank
    if (path.empty())
    {
      Position enemyPos = findClosestEnemyTank();
      if (!(enemyPos == Position(-1, -1))) // only compute path if enemy found
      {
        path = getActionsToEnemyTank(enemyPos);

        if (path.empty())
        {
          Position forwardPos = currentPos + Directions::directions().at(currentDir);
          if (isFree(forwardPos))
          {
            path.push(ActionRequest::MoveForward);
          }
          else
          {
            // Try rotating if can't move forward
            path.push(ActionRequest::RotateLeft90);
          }
        }
      }
      else
      {
        // No enemy found - explore randomly
        Position forwardPos = currentPos + Directions::directions().at(currentDir);
        if (isFree(forwardPos))
        {
          path.push(ActionRequest::MoveForward);
        }
        else
        {
          path.push(ActionRequest::RotateLeft90);
        }
      }
    }

    // Follow the planned path
    if (!path.empty())
    {
      ActionRequest nextAction = path.front();
      path.pop();
      return nextAction;
    }

    // Final fallback
    return ActionRequest::GetBattleInfo;
  }

  // Check if a cell is free of wall or mine
  bool TankAlgorithm_A::isFree(const Position &pos_other)
  {
    for (const auto &[player_idx, tank_idx, pos] : tanks)
    {
      if (pos == pos_other)
      {
        return false;
      }
    }
    return board.getWalls().find(pos_other) == board.getWalls().end() &&
           board.getMines().find(pos_other) == board.getMines().end();
  }

  // Check if tank can shoot an opponent
  bool TankAlgorithm_A::isShootPossible()
  {
    Position delta = Directions::directions().at(direction);
    Position current = pos + delta;
    while (current != this->pos)
    {
      for (const auto &[player_idx, tank_idx, tank_pos] : tanks)
      {
        if (current == tank_pos && player_idx == playerIndex)
        {
          return false; // Friendly in line of fire
        }
        else if (current == tank_pos && player_idx != playerIndex)
        {
          return true;
        }
      }
      current = current + delta;
    }
    return false;
  }

  int TankAlgorithm_A::manhattan(const Position &a, const Position &b)
  {
    return abs(a.x - b.x) + abs(a.y - b.y);
  }

  Position TankAlgorithm_A::findClosestEnemyTank()
  {
    int dist = INT_MAX;
    int curr_dist = INT_MAX;
    Position closest_pos = Position(-1, -1);
    for (const auto &[player_idx, tank_idx, tank_pos] : tanks)
    {
      if (this->playerIndex != player_idx)
      {
        curr_dist = manhattan(tank_pos, this->pos);
        if (curr_dist < dist)
        {
          closest_pos = tank_pos;
          dist = curr_dist;
        }
      }
    }
    return closest_pos;
  }

  // compute paths of shells on the board
  set<Position> TankAlgorithm_A::computeDangerZones()
  {
    set<Position> dangerZones;
    for (const auto &[shellPos, shellDir] : board.getShells())
    {
      if (manhattan(shellPos, pos) <= 2)
      {
        dangerZones.insert(shellPos);
      }
    }
    // Add mines to danger zones
    for (const auto &mine : board.getMines())
    {
      dangerZones.insert(mine);
    }
    return dangerZones;
  }
}
using Algorithm::TankAlgorithm_A;

REGISTER_TANK_ALGORITHM(TankAlgorithm_A)