#pragma once

#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <memory>

#include <vector>
#include "Units.h"
#include "Worker.h"

#include "raylib.h"
#include "MapManager.h"
#include "Timer.h"

#include "Building.h"

struct Map;
struct Building;
inline stringstream transcribeData(const string& _path);

constexpr int SCREEN_WIDTH = 1000;
constexpr int SCREEN_HEIGHT = 1200;
constexpr int TILE_SIZE = 10;

// Everything is a resource in the context of this game
struct ResourceTracker {
	// Resources:
	int treeCount = 0;
	int coalCount = 0;
	int ironOreCount = 0;
	int ironBarCount = 0;
	int swordCount = 0;
	// Buildings:
	int coalMileCount = 0;
	int smelterBuildingCount = 0;
	int armsmithForgeCount = 0;
	int trainingCamp = 0;
	// Units:
	int workerCount = 50;
	int scoutCount = 0;
	int coalMinerCount = 0;
	int armSmithCount = 0;
	int smelterCount = 0;
	int builderCount = 0;
	int soldierCount = 0;

	// this is really bad and lazy
	int workersDistributing = 0;
};

// Final hybrid-based design for units:
// Units act as autonomous agents and can affect internal state. They will do things like find Iron
// Houses are treated like permanent resources managed by the game manager (Only exception is that game manager might need to hard code the worker's targetPos)
// Game manager is responsible for training units whenever X amount of resources are available. This is done with a hard-coded fsm or a set of tailed if-else statements
// The benefit of this design amongst other things is that game manager can disable units from performing other actions while it's being converted

// Tldr: 
// Use exclusive if-else statements, event-based timers & ResourceTracker for deciding training of units
// Use 

enum UnitToTrain {
	EnumNone,
	EnumScout,
	EnumCoalMiner,
	EnumBuilder,
	EnumArmSmith,
	EnumSmelter,
	EnumSoldier
};

struct Event {
	UnitBase* unitPtr = nullptr;
	UnitToTrain unitToTrain;
	Timer trainTimer;

	Event(UnitBase* _unitPtr, UnitToTrain _unitType, float _trainTime) {
		unitPtr = _unitPtr;
		unitToTrain = _unitType;
		trainTimer.setNewTimer(_trainTime);
	}
};

// Purpose of Game manager: Manage everything game-related such as positions of things, , instantiations of other componenents etc
struct Game {
	stringstream mapData;
	// todo: turn these into smart pointers
	Map* gameMap = nullptr; 
	ResourceTracker* targetResourceCount = nullptr; // used so not all units do the same thing
	ResourceTracker* actualResourceCount = nullptr;
	std::vector<std::unique_ptr<UnitBase>> units;
	std::vector<Event> trainingUnits;

	int initialFormationColumns = 11;
	int xCount = 1;
	int yCount = 1;
	int baseXUnitSpawn = 80;
	int baseYUnitSpawn = 300;
	int spacing = 5;

	Game(int _initialUnits);

	// This effectively our brain for deciding units to train
	UnitToTrain getNextUnitToTrain();
	void startTrainingUnits(UnitToTrain unitType);
	void convertUnit(UnitBase* unitPtr, UnitToTrain unitType);
	void spawnBuildings();
	void updateTrainingUnits();
	void callUnits();
	void controlBuildings();
	void update();

	void debugText() {
		int fps = GetFPS();
		string fpsString = "FPS: " + to_string(fps);
		const char* fpsChar = fpsString.c_str();
		DrawText(fpsChar, 20, 1180, 12, RED);
	}
};
