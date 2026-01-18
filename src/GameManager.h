#pragma once

#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <memory>

#include <vector>
#include "Units.h"
#include "raylib.h"
#include "MapManager.h"
#include "TaskManager.h"
#include "Timer.h"

constexpr int SCREEN_WIDTH = 1000;
constexpr int SCREEN_HEIGHT = 1200;
constexpr int TILE_SIZE = 10;


// One resource tracker is instantly increased for 
// internal use so that we don't do 1 task 5 times.
// The other resource tracker is incremented after some timer count
struct ResourceTracker {
	// Resources:
	int treeCount = 0;
	int coalCount = 0;
	int ironOreCount = 0;
	int ironBarCount = 0;
	int swordCount = 0;
	// Buildings:
	int coalMileCount = 0;
	int armsForgeCount = 0;
	int smelterBuildingCount = 0;
	int trainingCamp = 0;
	// Units:
	int workerCount = 50;
	int scoutCount = 0;
	int coalMinerCount = 0;
	int armSmithCount = 0;
	int smelterCount = 0;
	int builderCount = 0;
	int soldierCount = 0;
};

// Final hybrid-based design for units:
// Units act as autonomous agents and can affect internal state. They will do things like find wood
// Houses are treated like permanent resources managed by the game manager (Only exception is that game manager might need to hard code the worker's targetPos)
// Game manager is responsible for training units whenever X amount of resources are available. This is done with a hard-coded fsm or a set of tailed if-else statements
// The benefit of this design amongst other things is that game manager can disable units from performing other actions while it's being converted

// Tldr: 
// Use exclusive if-else statements, event-based timers & ResourceTracker for deciding training of units
// Use 

enum UnitToTrain {
	EnumScout,
	EnumCoalMiner,
	EnumArmSmith,
	EnumSmelter,
	EnumBuilder,
	EnumSoldier
};

struct Event {
	UnitBase* unitPtr = nullptr;
	UnitToTrain unitToTrain;
	Timer trainTimer;

	Event(UnitBase* _unitPtr, UnitToTrain _unitType) {
		unitPtr = _unitPtr;
		unitToTrain = _unitType;
		trainTimer.setNewTimer(5);
	}
};

// Purpose of Game manager: Manage everything game-related such as positions of things, , instantiations of other componenents etc
struct Game {
	stringstream mapData;
	// todo: turn these into smart pointers
	Map* map = nullptr; 
	//TaskManager* tm = nullptr;
	ResourceTracker* resourceTrackerInternal = nullptr;
	ResourceTracker* resourceTrackerDisplay = nullptr;
	std::vector<std::unique_ptr<UnitBase>> units;
	std::vector<Event> trainingUnits;

	int initialFormationColumns = 11;
	int xCount = 1;
	int yCount = 1;
	int baseXUnitSpawn = 80;
	int baseYUnitSpawn = 200;
	int spacing = 10;

	Game(int _initialUnits) {
		mapData = transcribeData("..//mapData.txt");
		map = new Map(mapData, SCREEN_WIDTH, TILE_SIZE);
		resourceTrackerInternal = new ResourceTracker();
		resourceTrackerDisplay = new ResourceTracker();

		for (int i = 0; i < _initialUnits; i++) {
			if (xCount % initialFormationColumns == 0) {
				xCount = 1;
				yCount++;
			}

			units.push_back(std::make_unique<Worker>(
				baseXUnitSpawn + (spacing * xCount),
				baseYUnitSpawn + (spacing * yCount),
				map,
				resourceTrackerInternal));
			xCount++;

			units[i]->testTile();
		}
	}

	void startTrainingScouts() {
		for (auto& unit : units) {
			Worker* worker = dynamic_cast<Worker*>(unit.get());

			if (worker && !worker->isTraining) {
				worker->isTraining = true;
				Event trainEvent(worker, EnumScout);
				trainingUnits.push_back(trainEvent);
				std::cout << "Started training new scout" << std::endl;
				resourceTrackerInternal->scoutCount++;
				resourceTrackerInternal->workerCount--;
				break;
			}
		}
	}

	void updateTrainingUnits() {
		for (auto it = trainingUnits.begin(); it != trainingUnits.end();) {
			it->trainTimer.updateTimer();

			if (it->trainTimer.hasTimerEnded()) {
				trainScouts(it->unitPtr);
				it = trainingUnits.erase(it);
			}
			else {
				it++;
			}
		}
	}

	void trainScouts(UnitBase* workerPtr) {
		for (auto& unit : units) {
			if (unit.get() == workerPtr) {
				Vector2 tempPos = workerPtr->pos;
				unit = std::make_unique<Scout>(tempPos.x, tempPos.y, map, resourceTrackerInternal);
				resourceTrackerInternal->scoutCount++;
				resourceTrackerInternal->workerCount--;
				break;
			}
		}
	}

	void convertWorkerToScout() {
		for (auto& unit : units) {
			Worker* worker = dynamic_cast<Worker*>(unit.get());

			if (worker) {
				Vector2 tempPos = worker->pos;

				unit = std::make_unique<Scout>(tempPos.x, tempPos.y, map, resourceTrackerInternal);
				resourceTrackerDisplay->scoutCount++;
				resourceTrackerDisplay->workerCount--;
				break;
			}
		}
	}

	void callUnits() {
		for (auto& unit : units) {
			unit->renderUnit();

			if (!unit->isTraining) {
				unit->moveUnit();
				unit->moveUnitTowardsInternalGoal();
			}
		}

		if (!map->searchQueue.empty())
		{
			UnitBase* unit = map->searchQueue.front();
			map->searchQueue.pop();
			unit->calculateNewPath();
		}
	}

	void debugText() {
		int fps = GetFPS();
		string fpsString = to_string(fps);
		const char* fpsChar = fpsString.c_str();
		DrawText(fpsChar, 20, 1050, 24, GREEN);
	}

	void update() {
		debugText();

		if (resourceTrackerInternal->scoutCount < 5) {
			startTrainingScouts();
		}

		map->renderMap(SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE);
		callUnits();
		updateTrainingUnits();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
};
