#include "Simulator.h"
#include "AlgorithmRegistrar.h"
#include "common/GameManagerRegistration.h"
#include "GameManagerRegistrar.h"
#include "common/AbstractGameManager.h"
#include "common/GameResult.h"
#include "UserCommon/SatelliteViewImpl.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <unordered_set>

using namespace std;
namespace fs = std::filesystem;
using namespace UserCommon_318885712_208230862;

// Constructor
Simulator::Simulator(int argc, char *argv[])
{
    parseArguments(argc, argv);
}

// Destructor
Simulator::~Simulator()
{
    board.reset();
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    GameManagerRegistrar::getGameManagerRegistrar().clear();
    for (auto handle : soHandles)
    {
        if (handle)
        {
            dlclose(handle);
        }
    }
    soHandles.clear();
}

// Load Board
bool Simulator::loadBoard(const std::string &path)
{
    if (!fs::exists(path))
    {
        return false;
    }

    board = make_unique<GameBoard>(path);
    if (!board->isValid())
    {
        return false;
    }
    return true;
}

// Load so file
void Simulator::loadSharedObjectFromFile(const std::string &filePath, SharedObjectType type)
{
    namespace fs = std::filesystem;

    if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
        throw std::runtime_error("File does not exist or is not a regular file: " + filePath);

    if (fs::path(filePath).extension() != ".so")
        throw std::runtime_error("File is not a .so shared object: " + filePath);

    const std::string baseName = fs::path(filePath).stem().string();

    // Create registrar entry before loading
    if (type == SharedObjectType::Algorithm)
    {
        auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
        registrar.createAlgorithmFactoryEntry(baseName);
    }
    else
    {
        auto &gmRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
        gmRegistrar.createFactoryEntry(baseName);
    }

    void *handle = dlopen(filePath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle)
    {
        std::string err = dlerror();
        if (type == SharedObjectType::Algorithm)
            AlgorithmRegistrar::getAlgorithmRegistrar().removeLast();
        else
            GameManagerRegistrar::getGameManagerRegistrar().removeLast();

        throw std::runtime_error("Failed to load library: " + filePath + ". Error: " + err);
    }

    soHandles.push_back(handle);

    // Validate registration
    try
    {
        if (type == SharedObjectType::Algorithm)
        {
            AlgorithmRegistrar::getAlgorithmRegistrar().validateLastRegistration();
            std::cout << "Successfully registered Algorithm factories for: " << baseName << std::endl;
        }
        else
        {
            GameManagerRegistrar::getGameManagerRegistrar().validateLastRegistration();
            std::cout << "Successfully registered GameManager factory for: " << baseName << std::endl;
        }
    }
    catch (const AlgorithmRegistrar::BadRegistrationException &e)
    {
        std::cerr << "Bad Algorithm registration in " << e.name
                  << " | hasName: " << e.hasName
                  << ", hasPlayerFactory: " << e.hasPlayerFactory
                  << ", hasTankAlgorithmFactory: " << e.hasTankAlgorithmFactory
                  << std::endl;
        AlgorithmRegistrar::getAlgorithmRegistrar().removeLast();
    }
    catch (const GameManagerRegistrar::BadRegistrationException &e)
    {
        std::cerr << "Bad GameManager registration in " << e.name
                  << " | hasName: " << e.hasName
                  << ", hasFactory: " << e.hasFactory
                  << std::endl;
        GameManagerRegistrar::getGameManagerRegistrar().removeLast();
    }
}

// Load all so files from directory
void Simulator::loadSharedObjectsFromDirectory(const std::string &directoryPath, SharedObjectType type)
{
    namespace fs = std::filesystem;

    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath))
        throw std::runtime_error("Directory does not exist or is not a directory: " + directoryPath);

    size_t files_loaded_count = 0;

    for (const auto &entry : fs::directory_iterator(directoryPath))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".so")
            continue;

        try
        {
            loadSharedObjectFromFile(entry.path().string(), type);
            files_loaded_count++;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Skipping file " << entry.path() << " due to error: " << e.what() << std::endl;
        }
    }

    if (files_loaded_count == 0)
        throw std::runtime_error("Directory '" + directoryPath + "' does not contain any valid .so files.");
}

// Serialize the final game state into a string
std::string gameStateToString(const GameResult &result, size_t width, size_t height)
{
    std::ostringstream oss;
    for (size_t y = 0; y < height; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            oss << result.gameState->getObjectAt(x, y);
        }
        oss << '\n';
    }
    return oss.str();
}

// Run Comparative
void Simulator::runComparative(bool verbose)
{
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    auto &gmRegistrar = GameManagerRegistrar::getGameManagerRegistrar();

    loadSharedObjectFromFile(params.at("algorithm1"), SharedObjectType::Algorithm);
    loadSharedObjectFromFile(params.at("algorithm2"), SharedObjectType::Algorithm);
    loadSharedObjectsFromDirectory(params.at("game_managers_folder"), SharedObjectType::GameManager);

    std::string mapFile = params.at("game_map");
    if (!loadBoard(mapFile))
    {
        throw std::runtime_error("Failed to load or invalid board file: " + mapFile);
    }

    std::string outputFolder = params.at("game_managers_folder");
    std::string timeStr = getTimeString();
    std::string outputFile = outputFolder + "/comparative_results_" + timeStr + ".txt";

    std::ofstream out(outputFile);
    if (!out)
    {
        std::cerr << "Cannot create output file: " << outputFile << ", printing to screen.\n";
        out.basic_ios<char>::rdbuf(std::cout.rdbuf());
    }

    out << "game_map=" << mapFile << "\n";
    out << "algorithm1=" << params.at("algorithm1") << "\n";
    out << "algorithm2=" << params.at("algorithm2") << "\n\n";

    auto p1 = registrar.getAlgorithm(0).createPlayer(1, board->getWidth(), board->getHeight(), board->getMaxSteps(), 0);
    auto p2 = registrar.getAlgorithm(1).createPlayer(2, board->getWidth(), board->getHeight(), board->getMaxSteps(), 0);

    std::map<std::string, ComparativeResult> groupedResults;

    for (auto &gmEntry : gmRegistrar.getGM())
    {
        auto gm = gmEntry.create(verbose);
        auto sat = SatelliteViewImpl(*board, Position(-1, -1));
        auto mapName = fs::path(mapFile).stem().string();

        GameResult result = gm->run(
            board->getWidth(), board->getHeight(),
            dynamic_cast<SatelliteView &>(sat), mapName,
            board->getMaxSteps(), board->getNumShells(),
            *p1, registrar.getAlgorithm(0).name(),
            *p2, registrar.getAlgorithm(1).name(),
            registrar.getAlgorithm(0).getTankAlgorithmFactory(),
            registrar.getAlgorithm(1).getTankAlgorithmFactory());

        std::ostringstream sig;
        sig << result.winner << "|" << result.reason << "|" << result.rounds << "|"
            << gameStateToString(result, board->getWidth(), board->getHeight());

        std::string key = sig.str();
        groupedResults[key].result = move(result);
        groupedResults[key].gmNames.push_back(gmEntry.name);
        loadBoard(mapFile);
    }

    bool first = true;
    for (auto &[key, group] : groupedResults)
    {
        if (!first)
            out << "\n";
        first = false;

        for (size_t i = 0; i < group.gmNames.size(); i++)
        {
            if (i > 0)
                out << ",";
            out << group.gmNames[i];
        }
        out << "\n";

        if (group.result.winner == 0)
        {
            if (group.result.reason == GameResult::Reason::ALL_TANKS_DEAD)
                out << "Tie, reason: ALL_TANKS_DEAD" << "\n";
            else if (group.result.reason == GameResult::Reason::MAX_STEPS)
                out << "Tie, reason: MAX_STEPS" << "\n";
            else if (group.result.reason == GameResult::Reason::ZERO_SHELLS)
                out << "Tie, reason: ZERO_SHELLS" << "\n";
        }
        else
            out << "Player " << group.result.winner << " won, reason: ALL_TANKS_DEAD" << "\n";

        out << group.result.rounds << "\n";
        out << gameStateToString(group.result, board->getWidth(), board->getHeight()) << "\n";
    }
}

// Run Competition
void Simulator::runCompetition(bool verbose)
{
    auto &gmRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    loadSharedObjectsFromDirectory(params.at("algorithms_folder"), SharedObjectType::Algorithm);
    loadSharedObjectFromFile(params.at("game_manager"), SharedObjectType::GameManager);
    cout << registrar.count() << " algorithms registered.\n";
    if (registrar.count() < 2)
    {
        throw std::runtime_error("At least two algorithms are required for competition mode.");
    }

    string mapFolder = params.at("game_maps_folder");
    string gmSO = params.at("game_manager");
    string algFolder = params.at("algorithms_folder");

    string timeStr = getTimeString();
    string outputFile = algFolder + "/competition_" + timeStr + ".txt";
    ofstream out(outputFile);
    if (!out)
    {
        cerr << "Cannot create output file: " << outputFile << ", printing to screen.\n";
        out.basic_ios<char>::rdbuf(cout.rdbuf());
    }

    out << "game_maps_folder=" << mapFolder << "\n";
    out << "game_manager=" << gmSO << "\n\n";

    map<string, int> scores;
    for (size_t i = 0; i < registrar.count(); ++i)
        scores[registrar.getAlgorithm(i).name()] = 0;

    vector<string> maps;
    for (auto &p : fs::directory_iterator(mapFolder))
        maps.push_back(p.path().string());

    const auto &gmList = gmRegistrar.getGM();
    if (gmList.empty())
    {
        throw std::runtime_error("GameManager list is empty. Cannot retrieve factory.");
    }
    auto gmFactory = gmRegistrar.getGM()[0].getFactory();
    auto gm = gmFactory(verbose);

    size_t N = registrar.count();
    for (size_t k = 0; k < maps.size(); ++k)
    {
        if (!loadBoard(maps[k]))
        {
            cout << "Skipping invalid map: " << maps[k] << "\n";
            continue;
        }

        std::set<std::pair<size_t, size_t>> playedPairs;

        for (size_t i = 0; i < N; ++i)
        {
            size_t j = (i + 1 + k % (N - 1)) % N;
            if (i == j)
                continue;
            auto pair = std::minmax(i, j);
            if (playedPairs.count(pair))
                continue;

            auto p1 = registrar.getAlgorithm(i).createPlayer(1, board->getWidth(), board->getHeight(), board->getMaxSteps(), 0);
            auto p2 = registrar.getAlgorithm(j).createPlayer(2, board->getWidth(), board->getHeight(), board->getMaxSteps(), 0);
            auto sat = SatelliteViewImpl(*board, Position(-1, -1));
            auto mapName = fs::path(maps[k]).stem().string();

            GameResult result = gm->run(
                board->getWidth(), board->getHeight(),
                dynamic_cast<SatelliteView &>(sat), mapName,
                board->getMaxSteps(), board->getNumShells(),
                *p1, registrar.getAlgorithm(i).name(),
                *p2, registrar.getAlgorithm(j).name(),
                registrar.getAlgorithm(i).getTankAlgorithmFactory(),
                registrar.getAlgorithm(j).getTankAlgorithmFactory());

            playedPairs.insert(pair);

            if (result.winner == 0)
            {
                scores[registrar.getAlgorithm(i).name()] += 1;
                scores[registrar.getAlgorithm(j).name()] += 1;
            }
            else
            {
                scores[result.winner == 1 ? registrar.getAlgorithm(i).name() : registrar.getAlgorithm(j).name()] += 3;
            }
        }
    }

    vector<pair<string, int>> scoreVec(scores.begin(), scores.end());
    sort(scoreVec.begin(), scoreVec.end(), [](auto &a, auto &b)
         { return b.second < a.second; });

    for (auto &[name, score] : scoreVec)
        out << name << " " << score << "\n";

    out.close();
}

// Validate required parameters
void Simulator::validateRequiredParams()
{
    if (mode == RunMode::COMPARATIVE)
    {
        checkParamExists("game_map");
        checkParamExists("game_managers_folder");
        checkParamExists("algorithm1");
        checkParamExists("algorithm2");
    }
    else if (mode == RunMode::COMPETITION)
    {
        checkParamExists("game_maps_folder");
        checkParamExists("game_manager");
        checkParamExists("algorithms_folder");
    }
}

// Check if parameter exists
void Simulator::checkParamExists(const string &paramName)
{
    if (params.find(paramName) == params.end())
        throw invalid_argument("Missing required parameter: " + paramName);
}

// Get current time string
string Simulator::getTimeString() const
{
    auto t = chrono::system_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(t.time_since_epoch()).count();
    return to_string(ms);
}

// Parse command line arguments
void Simulator::parseArguments(int argc, char *argv[])
{
    if (argc < 2)
        throw std::invalid_argument("Not enough arguments provided.");

    bool mode_updated = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-comparative")
        {
            mode = RunMode::COMPARATIVE;
            mode_updated = true;
        }
        else if (arg == "-competition")
        {
            mode = RunMode::COMPETITION;
            mode_updated = true;
        }
        else if (arg == "-verbose")
            verbose = true;
        else if (arg.find("num_threads=") == 0)
        {
            size_t eqPos = arg.find('=');
            std::string value = arg.substr(eqPos + 1);
            try
            {
                int threads = std::stoi(value);
                if (threads < 1)
                {
                    throw std::invalid_argument("num_threads must be at least 1");
                }
                if (threads == 2)
                {
                    numThreads = 1;
                    std::cout << "Warning: num_threads=2 is not allowed. Using single thread instead.\n";
                }
                else
                {
                    numThreads = threads;
                }
            }
            catch (const std::exception &)
            {
                throw std::invalid_argument("Invalid num_threads value: " + value);
            }
        }
        else
        {
            size_t eqPos = arg.find('=');
            if (eqPos == std::string::npos)
                throw std::invalid_argument("Invalid argument format: " + arg);
            std::string key = arg.substr(0, eqPos);
            if (key != "game_map" && key != "game_managers_folder" && key != "algorithm1" && key != "algorithm2" && key != "game_maps_folder" && key != "game_manager" && key != "algorithms_folder")
            {
                throw std::invalid_argument("Unsupported argument:" + key);
            }
            std::string value = arg.substr(eqPos + 1);
            params[key] = value;
        }
    }
    if (!mode_updated)
    {
        throw std::invalid_argument("missing -comparative or -competition");
    }
    validateRequiredParams();
}

// Run the simulator
void Simulator::run(bool verbose)
{
    if (mode == RunMode::COMPETITION)
    {
        if (numThreads == 1)
        {
            runCompetition(verbose);
        }
        else
        {
            runCompetitionThreaded(verbose);
        }
    }
    else if (mode == RunMode::COMPARATIVE)
    {
        if (numThreads == 1)
        {
            runComparative(verbose);
        }
        else
        {
            runComparativeThreaded(verbose);
        }
    }
    else
    {
        throw std::runtime_error("Unknown run mode");
    }
}

// Determine optimal thread count
int Simulator::getOptimalThreadCount(size_t totalTasks) const
{
    if (numThreads == 1 || totalTasks <= 1)
    {
        return 1;
    }

    // Don't create more threads than tasks
    int optimalThreads = std::min(static_cast<int>(totalTasks), numThreads);

    // Never allow exactly 2 threads total
    if (optimalThreads == 2)
    {
        optimalThreads = 1;
    }

    return optimalThreads;
}

// Create a new GameBoard instance
std::unique_ptr<GameBoard> Simulator::createGameBoard(const std::string &mapFile) const
{
    auto gameBoard = std::make_unique<GameBoard>(mapFile);
    if (!gameBoard->isValid())
    {
        throw std::runtime_error("Failed to load or invalid board file: " + mapFile);
    }
    return gameBoard;
}

// Competition task structure
void Simulator::runCompetitionThreaded(bool verbose)
{
    auto &gmRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
    (void)gmRegistrar;
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();

    // Load shared objects
    loadSharedObjectsFromDirectory(params.at("algorithms_folder"), SharedObjectType::Algorithm);
    loadSharedObjectFromFile(params.at("game_manager"), SharedObjectType::GameManager);
    std::cout << registrar.count() << " algorithms registered.\n";
    if (registrar.count() < 2)
    {
        throw std::runtime_error("At least two algorithms are required for competition mode.");
    }

    std::string mapFolder = params.at("game_maps_folder");
    std::string gmSO = params.at("game_manager");
    std::string algFolder = params.at("algorithms_folder");

    std::string timeStr = getTimeString();
    std::string outputFile = algFolder + "/competition_" + timeStr + ".txt";
    std::ofstream out(outputFile);
    if (!out)
    {
        std::cerr << "Cannot create output file: " << outputFile << ", printing to screen.\n";
        out.basic_ios<char>::rdbuf(std::cout.rdbuf());
    }

    out << "game_maps_folder=" << mapFolder << "\n";
    out << "game_manager=" << gmSO << "\n\n";

    // Initialize scores
    std::map<std::string, int> scores;
    for (size_t i = 0; i < registrar.count(); ++i)
    {
        scores[registrar.getAlgorithm(i).name()] = 0;
    }

    // Get all map files
    std::vector<std::string> maps;
    for (auto &p : std::filesystem::directory_iterator(mapFolder))
    {
        maps.push_back(p.path().string());
    }

    // Create task queue
    std::queue<GameTask> taskQueue;
    std::mutex queueMutex;
    size_t taskId = 0;

    size_t N = registrar.count();
    for (size_t k = 0; k < maps.size(); ++k)
    {
        std::set<std::pair<size_t, size_t>> playedPairs;

        for (size_t i = 0; i < N; ++i)
        {
            size_t j = (i + 1 + k % (N - 1)) % N;
            if (i == j)
                continue;

            auto pair = std::minmax(i, j);
            if (playedPairs.count(pair))
                continue;

            taskQueue.push({i, j, maps[k], taskId++});
            playedPairs.insert(pair);
        }
    }

    size_t totalTasks = taskQueue.size();
    int actualThreads = getOptimalThreadCount(totalTasks);

    if (actualThreads == 1)
    {
        runCompetition(verbose);
        return;
    }
    std::vector<std::thread> workers;
    for (int i = 0; i < actualThreads; ++i)
    {
        workers.emplace_back(&Simulator::competitionWorker, this,
                             std::cref(maps), std::ref(taskQueue),
                             std::ref(queueMutex), std::ref(scores), verbose);
    }

    // Wait for all threads to complete
    for (auto &worker : workers)
    {
        worker.join();
    }

    // Write results
    std::vector<std::pair<std::string, int>> scoreVec(scores.begin(), scores.end());
    std::sort(scoreVec.begin(), scoreVec.end(), [](const auto &a, const auto &b)
              { return b.second < a.second; });

    for (const auto &[name, score] : scoreVec)
    {
        out << name << " " << score << "\n";
    }

    out.close();
}

void Simulator::competitionWorker(
    const std::vector<std::string> &maps,
    std::queue<GameTask> &taskQueue,
    std::mutex &queueMutex,
    std::map<std::string, int> &scores,
    bool verbose)
{
    (void)maps;
    auto &gmRegistrar = GameManagerRegistrar::getGameManagerRegistrar();
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();

    // Each thread creates its own GameManager instance
    const auto &gmList = gmRegistrar.getGM();
    if (gmList.empty())
    {
        throw std::runtime_error("GameManager list is empty. Cannot retrieve factory.");
    }
    auto gmFactory = gmRegistrar.getGM()[0].getFactory();
    auto gm = gmFactory(verbose);

    while (true)
    {
        GameTask task;

        // Get next task from queue
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (taskQueue.empty())
            {
                break; // No more tasks
            }
            task = taskQueue.front();
            taskQueue.pop();
        }

        try
        {
            // Load board for this thread
            auto threadBoard = createGameBoard(task.mapFile);

            // Create players
            auto p1 = registrar.getAlgorithm(task.player1_idx).createPlayer(1, threadBoard->getWidth(), threadBoard->getHeight(), threadBoard->getMaxSteps(), 0);
            auto p2 = registrar.getAlgorithm(task.player2_idx).createPlayer(2, threadBoard->getWidth(), threadBoard->getHeight(), threadBoard->getMaxSteps(), 0);
            auto sat = UserCommon_318885712_208230862::SatelliteViewImpl(*threadBoard, Position(-1, -1));
            auto mapName = fs::path(task.mapFile).stem().string();

            GameResult result = gm->run(
                threadBoard->getWidth(), threadBoard->getHeight(),
                dynamic_cast<SatelliteView &>(sat), mapName,
                threadBoard->getMaxSteps(), threadBoard->getNumShells(),
                *p1, registrar.getAlgorithm(task.player1_idx).name(),
                *p2, registrar.getAlgorithm(task.player2_idx).name(),
                registrar.getAlgorithm(task.player1_idx).getTankAlgorithmFactory(),
                registrar.getAlgorithm(task.player2_idx).getTankAlgorithmFactory());

            // Update scores
            {
                std::lock_guard<std::mutex> lock(resultsMutex);
                if (result.winner == 0)
                {
                    scores[registrar.getAlgorithm(task.player1_idx).name()] += 1;
                    scores[registrar.getAlgorithm(task.player2_idx).name()] += 1;
                }
                else
                {
                    std::string winnerName = result.winner == 1 ? registrar.getAlgorithm(task.player1_idx).name() : registrar.getAlgorithm(task.player2_idx).name();
                    scores[winnerName] += 3;
                }
            }
            completedTasks++;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in worker thread for task " << task.taskId
                      << ": " << e.what() << std::endl;
        }
    }
}

void Simulator::runComparativeThreaded(bool verbose)
{
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    (void)registrar;
    auto &gmRegistrar = GameManagerRegistrar::getGameManagerRegistrar();

    loadSharedObjectFromFile(params.at("algorithm1"), SharedObjectType::Algorithm);
    loadSharedObjectFromFile(params.at("algorithm2"), SharedObjectType::Algorithm);
    loadSharedObjectsFromDirectory(params.at("game_managers_folder"), SharedObjectType::GameManager);

    std::string mapFile = params.at("game_map");
    loadBoard(mapFile);

    std::string outputFolder = params.at("game_managers_folder");
    std::string timeStr = getTimeString();
    std::string outputFile = outputFolder + "/comparative_results_" + timeStr + ".txt";

    std::ofstream out(outputFile);
    if (!out)
    {
        std::cerr << "Cannot create output file: " << outputFile << ", printing to screen.\n";
        out.basic_ios<char>::rdbuf(std::cout.rdbuf());
    }

    out << "game_map=" << mapFile << "\n";
    out << "algorithm1=" << params.at("algorithm1") << "\n";
    out << "algorithm2=" << params.at("algorithm2") << "\n\n";

    // Create GameManager queue
    std::queue<std::string> gmQueue;
    std::mutex queueMutex;

    for (auto &gmEntry : gmRegistrar.getGM())
    {
        gmQueue.push(gmEntry.name);
    }

    size_t totalGMs = gmQueue.size();
    int actualThreads = getOptimalThreadCount(totalGMs);

    if (actualThreads == 1)
    {
        runComparative(verbose);
        return;
    }

    // Shared results map
    std::map<std::string, ComparativeResult> groupedResults;

    std::vector<std::thread> workers;
    for (int i = 0; i < actualThreads; ++i)
    {
        workers.emplace_back(&Simulator::comparativeWorker, this,
                             std::ref(gmQueue), std::ref(queueMutex),
                             std::ref(groupedResults), mapFile, verbose);
    }

    // Wait for all threads to complete
    for (auto &worker : workers)
    {
        worker.join();
    }

    bool first = true;
    for (auto &[key, group] : groupedResults)
    {
        if (!first)
            out << "\n";
        first = false;

        for (size_t i = 0; i < group.gmNames.size(); i++)
        {
            if (i > 0)
                out << ",";
            out << group.gmNames[i];
        }
        out << "\n";

        if (group.result.winner == 0)
        {
            if (group.result.reason == GameResult::Reason::ALL_TANKS_DEAD)
                out << "Tie, reason: ALL_TANKS_DEAD\n";
            else if (group.result.reason == GameResult::Reason::MAX_STEPS)
                out << "Tie, reason: MAX_STEPS\n";
            else if (group.result.reason == GameResult::Reason::ZERO_SHELLS)
                out << "Tie, reason: ZERO_SHELLS\n";
        }
        else
        {
            out << "Player " << group.result.winner << " won, reason: ALL_TANKS_DEAD\n";
        }

        out << group.result.rounds << "\n";
        out << gameStateToString(group.result, board->getWidth(), board->getHeight()) << "\n";
    }
}

void Simulator::comparativeWorker(
    std::queue<std::string> &gmQueue,
    std::mutex &queueMutex,
    std::map<std::string, ComparativeResult> &groupedResults,
    const std::string &mapFile,
    bool verbose)
{
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    auto &gmRegistrar = GameManagerRegistrar::getGameManagerRegistrar();

    while (true)
    {
        std::string gmName;

        // Get next GameManager from queue
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (gmQueue.empty())
            {
                break; // No more GameManagers to process
            }
            gmName = gmQueue.front();
            gmQueue.pop();
        }

        try
        {
            // Load board for this thread
            auto threadBoard = createGameBoard(mapFile);

            // Create players
            auto p1 = registrar.getAlgorithm(0).createPlayer(
                1, threadBoard->getWidth(), threadBoard->getHeight(),
                threadBoard->getMaxSteps(), 0);
            auto p2 = registrar.getAlgorithm(1).createPlayer(
                2, threadBoard->getWidth(), threadBoard->getHeight(),
                threadBoard->getMaxSteps(), 0);

            auto gmEntry = std::find_if(gmRegistrar.getGM().begin(), gmRegistrar.getGM().end(),
                                        [&gmName](const auto &entry)
                                        { return entry.name == gmName; });
            if (gmEntry == gmRegistrar.getGM().end())
            {
                throw std::runtime_error("GameManager not found: " + gmName);
            }

            auto gm = gmEntry->create(verbose);
            auto sat = UserCommon_318885712_208230862::SatelliteViewImpl(*threadBoard, Position(-1, -1));
            auto mapName = fs::path(mapFile).stem().string();
            GameResult result = gm->run(
                threadBoard->getWidth(), threadBoard->getHeight(),
                dynamic_cast<SatelliteView &>(sat), mapName,
                threadBoard->getMaxSteps(), threadBoard->getNumShells(),
                *p1, registrar.getAlgorithm(0).name(),
                *p2, registrar.getAlgorithm(1).name(),
                registrar.getAlgorithm(0).getTankAlgorithmFactory(),
                registrar.getAlgorithm(1).getTankAlgorithmFactory());

            std::ostringstream sig;
            sig << result.winner << "|" << result.reason << "|" << result.rounds << "|"
                << gameStateToString(result, threadBoard->getWidth(), threadBoard->getHeight());

            std::string key = sig.str();

            {
                std::lock_guard<std::mutex> lock(resultsMutex);
                groupedResults[key].result = std::move(result);
                groupedResults[key].gmNames.push_back(gmName);
            }

            completedTasks++;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in comparative worker thread for GM " << gmName
                      << ": " << e.what() << std::endl;
        }
    }
}
