#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include "common/TankAlgorithm.h"
#include "common/Player.h"
#include "GameBoard.h"
#include "common/GameResult.h"

enum class RunMode
{
    COMPETITION,
    COMPARATIVE,
    UNKNOWN
};

enum class SharedObjectType
{
    Algorithm,
    GameManager
};

struct ComparativeResult
{
    std::vector<std::string> gmNames;
    GameResult result;
};

struct GameTask
{
    size_t player1_idx;
    size_t player2_idx;
    std::string mapFile;
    size_t taskId;
};

struct GameTaskResult
{
    size_t taskId;
    size_t player1_idx;
    size_t player2_idx;
    std::string mapFile;
    GameResult result;
};

class Simulator
{
public:
    Simulator(int argc, char *argv[]);

    ~Simulator();

    void run(bool verbose);

    RunMode getMode() const { return mode; }
    bool isVerbose() const { return verbose; }
    const std::map<std::string, std::string> &getParams() const { return params; }

private:
    RunMode mode;
    bool verbose = false;
    int numThreads = 1;

    std::map<std::string, std::string> params;
    std::unique_ptr<UserCommon::GameBoard> board;
    std::vector<void *> soHandles;

    struct AlgorithmEntry
    {
        std::string name;
        std::unique_ptr<Player> player;
        TankAlgorithmFactory tankAlgorithm;
    };

    std::vector<AlgorithmEntry> loadedAlgorithms;

    std::mutex resultsMutex;
    std::atomic<size_t> completedTasks{0};

    void parseArguments(int argc, char *argv[]);
    void validateRequiredParams();
    void checkParamExists(const std::string &paramName);
    std::string getTimeString() const;

    bool loadBoard(const std::string &path);
    void loadSharedObjectFromFile(const std::string &filePath, SharedObjectType type);
    void loadSharedObjectsFromDirectory(const std::string &directoryPath, SharedObjectType type);

    void runCompetition(bool verbose);

    void runComparative(bool verbose);

    void runCompetitionThreaded(bool verbose);
    void runComparativeThreaded(bool verbose);

    void competitionWorker(
        const std::vector<std::string> &maps,
        std::queue<GameTask> &taskQueue,
        std::mutex &queueMutex,
        std::map<std::string, int> &scores,
        bool verbose);

    void comparativeWorker(
        std::queue<std::string> &gmQueue,
        std::mutex &queueMutex,
        std::map<std::string, ComparativeResult> &groupedResults,
        const std::string &mapFile,
        bool verbose);

    int getOptimalThreadCount(size_t totalTasks) const;

    std::unique_ptr<UserCommon::GameBoard> createGameBoard(const std::string &mapFile) const;
};
