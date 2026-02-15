
/* 
	Design overview (Ordered from top to bottom by priority): 

	// Managers:
	Game manager: The master manager which initializes everything and glues everything together.
	Map manager: Keeps state of fog of war, maps every tile with some itemization, utilizes file manager upon compilation etc
		Every tile is stored as a grid with some data of its state. 
		The map manager should also provide helper functions such as: findNearestTree(&agent agentPos) which returns a tile.
		It should also use a similar system as assignment 2 in case that wasn't clear enough (With the addition of explicit tiles)
	Time manager: Responsible for changing the tick rate. All components should factor in this via singleton pattern.
		It should essentially halt the while() loop until X amount of time has passed.
	Task manager: Responsible for keeping track of & manipulating resources, reference map manager, buildings. Should also implement an FSM.
		Might also want to implement a priority queue which returns more urgent tasks, which agent will read from before their own FSM.
	
	// Units
	Agents independent thinkers, however to avoid clashing certain tasks like build buildings are done via task manager (Hive mind like)
	Unit base class: Incorporates things like steering, collision & collision avoidance, sprite rendering, time control, idle state

	Scouts: Only have insight of unvisited tiles. Should maybe use dijkstra since we don't have a goal to search for.
	Craftsmen: Have a reference to the task manager's 
	Workers: Have insight of visited tiles and reference to resources so they know what to prioritize
	Soldiers: Idk, just stand there to look cool?
	Builder: Takes input from task manager which buildings are being built. Otherwise afk.

	Workers, soldiers and craftsmen inherit from Unit
	Coal miner, arm smith, smelter and builder all inherit from craftsmen 
	(Assuming craftsman can be generalized. Otherwise they also inherit from untit)

	// Buildings:
	Are instantly appended to building count in a particular frame to avoid clashing
	Implements collision
	Enables the creation of either materials or soldiers via some simplified queue system (since each building can only do 1 thing)

	Rendering asset pack if i find time: https://kenney.nl/assets/1-bit-pack
*/

#include "raylib.h"
#include "resource_dir.h"	
#include <iostream>
#include <fstream>

#include "FileRead.h"

#include "GameManager.h"
#include "MapManager.h"

#include <chrono>

// Next Todo (Remove this when done): 
// Create scout class (It's dead simple since it's always just searching the map) that can search the map as well as fog of war for scouts to challenge

// Idea: Game is 1000x1000x with each tile taking up 10 pixels for width and height
// since game is a 100x100x grid which leaves us 200 pixels for displaying game state

using namespace std;

int main ()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Raylib");
	SearchAndSetResourceDir("resources");
	SetTargetFPS(65);
	
	//stringstream mapData = transcribeData("..//mapData.txt");
	//Map map(mapData, SCREEN_WIDTH, TILE_SIZE);

	bool soldierGoalReached = false;

	Game game(&soldierGoalReached);

	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;

	auto t1 = high_resolution_clock::now();

	while (!WindowShouldClose() && !soldierGoalReached)
	{
		BeginDrawing();
		ClearBackground(BLACK);
		game.update();
		EndDrawing();
	}

	auto t2 = high_resolution_clock::now();
	auto ms_int = duration_cast<milliseconds>(t2 - t1);
	duration<double, std::milli> ms_double = t2 - t1;

	std::cout << "Final time to make 20 soldiers (In seconds): " << (ms_double.count() / 1000) * resourceParameters.timeControl;

	CloseWindow();

	return 0;
}
