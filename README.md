# Tank Game Simulator – Assignment 3

This project implements the tank battle simulator as required in EX3.
The simulator supports both **competition mode** and **comparative mode**, with dynamic loading of player algorithms and game managers.

## Implementation Details
- `Simulator.cpp` – Main entry point, handles arguments, multi-threading and execution.
The simulator loads game boards and dynamically registers algorithm and game manager shared libraries, then runs games according to the selected mode. In competition mode it schedules all algorithm pairs across all maps (optionally using multiple threads), while in comparative mode it runs the same map and algorithms under different game managers, collecting and outputting the results.
- `GameManager_318885712_208230862.cpp` – The game manager controls the simulation by applying tank actions, moving shells, and resolving collisions with walls, mines, or other tanks.
It enforces the game rules, tracks win conditions, and logs the sequence of actions until the match ends.
- `TankAlgorithm_318885712_208230862.cpp` - Our tank algorithm combines shooting and danger avoidance with pathfinding using BFS to reach the nearest enemy tank.
It dynamically updates battlefield information, avoids mines and shells, shoots when possible, and adapts its movement accordingly.


## Compilation & Run
## Compilation

You can compile the entire project at once by running:

```bash
make
```
Alternatively, each directory contains its own Makefile, so you can compile just that specific part of the project by running make inside the desired directory.

Run with:
Comparative run: 
```bash
./simulator_<submitter_ids> -comparative game_map=<game_map_filename> game_managers_folder=<game_managers_folder> algorithm1=<algorithm_so_filename> algorithm2=<algorithm_so_filename> [num_threads=<num>] [-verbose]
```

Competition run: 
```bash
./simulator_<submitter_ids> -competition game_maps_folder=<game_maps_folder> game_manager=<game_manager_so_filename> algorithms_folder=<algorithms_folder> [num_threads=<num>] [-verbose]
```
