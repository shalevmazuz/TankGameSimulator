#include "GameBoard.h"
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

namespace UserCommon
{

  using namespace std;

  GameBoard::GameBoard()
  {
  }

  GameBoard::GameBoard(size_t width, size_t height, size_t maxSteps,
                       set<Position> &walls, set<Position> &mines,
                       vector<tuple<int, int, Position>> &&tanks)
      : is_valid(true), height(height), width(width), maxSteps(maxSteps),
        walls(walls), mines(mines), name("name")
  {
    this->tanks = std::move(tanks);
    Position::height = height;
    Position::width = width;
  }

  GameBoard::GameBoard(const string &filename)
  {
    ifstream infile(filename);
    name = fs::path(filename).stem().string();
    if (!infile)
    {
      cerr << "Error: Cannot open file: " << filename << endl;
      is_valid = false;
      return;
    }

    string line;

    // Line 1: map name/description (ignored)
    if (!getline(infile, line))
    {
      cerr << "Error: Missing map name/description line." << endl;
      is_valid = false;
      return;
    }

    // Line 2: MaxSteps = <NUM>
    size_t max_steps = 0;
    if (!getline(infile, line) || sscanf(line.c_str(), "MaxSteps = %zu", &max_steps) != 1)
    {
      cerr << "Error: Could not parse MaxSteps line." << endl;
      is_valid = false;
      return;
    }
    maxSteps = max_steps;

    // Line 3: NumShells = <NUM>
    size_t num_shells = 0;
    if (!getline(infile, line) || sscanf(line.c_str(), "NumShells = %zu", &num_shells) != 1)
    {
      cerr << "Error: Could not parse NumShells line." << endl;
      is_valid = false;
      return;
    }
    numShells = num_shells;

    // Line 4: Rows = <NUM>
    int height = 0;
    if (!getline(infile, line) || sscanf(line.c_str(), "Rows = %d", &height) != 1)
    {
      cerr << "Error: Could not parse Rows line." << endl;
      is_valid = false;
      return;
    }

    // Line 5: Cols = <NUM>
    int width = 0;
    if (!getline(infile, line) || sscanf(line.c_str(), "Cols = %d", &width) != 1)
    {
      cerr << "Error: Could not parse Cols line." << endl;
      is_valid = false;
      return;
    }

    set<Position> temp_walls, temp_mines;
    vector<tuple<int, int, Position>> temp_tanks;
    vector<string> errors;
    int p1Tanks, p2Tanks = 0;

    for (int y = 0; y < height; ++y)
    {
      if (!getline(infile, line))
      {
        errors.push_back("Missing row at y=" + to_string(y));
        continue;
      }

      if (line.size() < static_cast<size_t>(width))
      {
        errors.push_back("Row at y=" + to_string(y) + " is shorter than width. Filling with spaces.");
        line.append(width - line.size(), ' ');
      }

      for (int x = 0; x < width; ++x)
      {
        Position p{x, y};
        char ch = line[x];
        switch (ch)
        {
        case '#':
          temp_walls.insert(p);
          break;
        case '@':
          temp_mines.insert(p);
          break;
        case '1':
        {
          temp_tanks.emplace_back(1, p1Tanks, p);
          ++p1Tanks;
          break;
        }

        case '2':
        {
          temp_tanks.emplace_back(2, p2Tanks, p);
          ++p2Tanks;
          break;
        }
        case ' ':
          break;
        default:
          errors.push_back("Unrecognized character '" + string(1, ch) + "' at x=" + to_string(x) + ", y=" + to_string(y) + ". Treated as empty.");
          break;
        }
      }
    }

    this->width = width;
    this->height = height;
    this->walls = std::move(temp_walls);
    this->mines = std::move(temp_mines);
    this->tanks = std::move(temp_tanks);

    if (!errors.empty())
    {
      ofstream errfile("input_errors_" + name + ".txt");
      for (const auto &e : errors)
        errfile << e << endl;
    }
    is_valid = true;
  }

  GameBoard::~GameBoard() = default;
}