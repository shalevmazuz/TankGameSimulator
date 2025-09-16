#include "Simulator.h"
int main(int argc, char **argv)
{
    try
    {
        Simulator sim(argc, argv);
        sim.run(sim.isVerbose());
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
