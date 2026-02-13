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
struct Worker;
struct Building;
struct TrainingCamp;
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
	int ironArrowCount = 0;
	int ironSwordCount = 00;
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
	int woodInCoalMile = 0;
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

	std::vector<Worker*> soldiersToTrain;

	TrainingCamp* tc = nullptr;

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
		DrawText(fpsString.c_str(), 20, 1180, 12, RED);

		string workerCount = "Worker count: " + to_string(targetResourceCount->workerCount);
		DrawText(workerCount.c_str(), 100, 1180, 12, RED);

		string scoutCount = "Scout count: " + to_string(targetResourceCount->scoutCount);
		DrawText(scoutCount.c_str(), 225, 1160, 12, BLUE);

		string builderCount = "Builder count: " + to_string(targetResourceCount->builderCount);
		DrawText(builderCount.c_str(), 325, 1180, 12, ORANGE);

		string coalMinercount = "Coal Operator count: " + to_string(targetResourceCount->coalMinerCount);
		DrawText(coalMinercount.c_str(), 425, 1180, 12, GRAY);

		string soldiercount = "Soldier count: " + to_string(targetResourceCount->soldierCount);
		DrawText(soldiercount.c_str(), 100, 1160, 12, GOLD);

		string smelterCount = "Smelter count: " + to_string(targetResourceCount->smelterCount);
		DrawText(smelterCount.c_str(), 225, 1180, 12, BLUE);
	}
};
