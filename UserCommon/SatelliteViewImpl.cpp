#include "SatelliteViewImpl.h"
#include "GameBoard.h"
#include <cmath>
#include <memory>

namespace UserCommon
{
  using namespace std;

  SatelliteViewImpl::SatelliteViewImpl(const GameBoard &board, Position tankPos)
      : tankPos(tankPos)
  {
    this->height = board.getHeight();
    this->width = board.getWidth();
    this->walls = board.getWalls();
    this->mines = board.getMines();
    this->weakenedWalls = board.getWeakenedWalls();
    this->tanks = board.getTanks();
    this->shells = board.getShells();
  }

  char SatelliteViewImpl::getObjectAt(size_t x, size_t y) const
  {
    Position pos = Position(x, y);
    if (y > this->height || x > this->width)
    {
      return '&';
    }
    if (pos == tankPos)
    {
      return '%';
    }
    for (const auto &shell : this->shells)
    {
      const Position &shell_pos = shell.first;
      if (pos == shell_pos)
      {
        return '*';
      }
    }
    if (this->walls.find(pos) != this->walls.end())
    {
      return '#';
    }
    else if (this->mines.find(pos) != this->mines.end())
    {
      return '@';
    }
    else
      for (const auto &[player_idx, tank_idx, tank_pos] : this->tanks)
      {
        if (pos == tank_pos && player_idx == 1)
        {
          return '1';
        }
        else if (pos == tank_pos && player_idx == 2)
        {
          return '2';
        }
      }
    return ' ';
  }
}