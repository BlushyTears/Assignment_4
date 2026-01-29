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
	EnumNone,
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
	Map* map = nullptr; 
	ResourceTracker* targetResourceCount = nullptr; // used so not all units do the same thing
	ResourceTracker* actualResourceCount = nullptr;
	std::vector<std::unique_ptr<UnitBase>> units;
	std::vector<Event> trainingUnits;
	std::vector<Building*> buildings;

	int initialFormationColumns = 11;
	int xCount = 1;
	int yCount = 1;
	int baseXUnitSpawn = 80;
	int baseYUnitSpawn = 300;
	int spacing = 5;

	Game(int _initialUnits) {
		mapData = transcribeData("..//mapData.txt");
		map = new Map(mapData, SCREEN_WIDTH, TILE_SIZE);
		targetResourceCount = new ResourceTracker();
		actualResourceCount = new ResourceTracker();

		for (int i = 0; i < _initialUnits; i++) {
			if (xCount % initialFormationColumns == 0) {
				xCount = 1;
				yCount++;
			}

			units.push_back(std::make_unique<Worker>(
				baseXUnitSpawn + (spacing * xCount),
				baseYUnitSpawn + (spacing * yCount),
				map,
				targetResourceCount,
				&units));
			xCount++;

			units[i]->testTile();
		}

		// Create buildings
		Vector2 pos = { (float)baseXUnitSpawn * 2, (float)baseYUnitSpawn };
		CoalMile* cm = new CoalMile({ pos }, targetResourceCount, TILE_SIZE);
		buildings.push_back(cm);
	}

	// This effectively our brain for deciding units to train
	UnitToTrain getNextUnitToTrain() {
		if (targetResourceCount->scoutCount < 5) {
			targetResourceCount->scoutCount++;
			return EnumScout;
		}
		else if (targetResourceCount->coalMinerCount < 0) {
			targetResourceCount->coalMinerCount++;
			return EnumCoalMiner;
		}
		return EnumNone;
	}

	void startTrainingUnits(UnitToTrain unitType) {
		// We're done training units
		if (unitType == EnumNone) {
			return;
		}
		for (auto& unit : units) {
			Worker* worker = dynamic_cast<Worker*>(unit.get());

			float trainTime = 5.0f;
			switch (unitType) {
				case EnumScout: trainTime = 3.0f; break;
				case EnumCoalMiner: trainTime = 3.0f; break;
			}

			if (worker && !worker->isTraining) {
				worker->isTraining = true;
				Event trainEvent(worker, unitType, trainTime);
				trainingUnits.push_back(trainEvent);
				break;
			}
		}
	}

	void convertUnit(UnitBase* unitPtr, UnitToTrain unitType) {
		for (auto& unit : units) {
			if (unit.get() == unitPtr) {
				Vector2 tempPos = unitPtr->pos;
				actualResourceCount->workerCount--;

				switch (unitType) {
					case EnumScout:
						unit = std::make_unique<Scout>(tempPos.x, tempPos.y, map, targetResourceCount, &units);
						actualResourceCount->scoutCount++;
						break;
					case EnumCoalMiner:
						unit = std::make_unique<CoalWorker>(tempPos.x, tempPos.y, map, targetResourceCount, &units);
						actualResourceCount->coalMinerCount++;
						break;
					case EnumNone:
						break;
				}
				unit->isTraining = false;
				break;
			}
		}
	}

	void updateTrainingUnits() {
		for (auto it = trainingUnits.begin(); it != trainingUnits.end();) {
			it->trainTimer.updateTimer();

			if (it->trainTimer.hasTimerEnded()) {
				convertUnit(it->unitPtr, it->unitToTrain);
				it = trainingUnits.erase(it);
			}
			else {
				it++;
			}
		}
	}

	void callUnits() {
		int idx = 0;
		for (auto& unit : units) {
			idx++;
			unit->renderUnit();

			if (!unit->isTraining) {
				unit->commandUnit();
				unit->moveUnitTowardsInternalGoal();
			}
		}

		if (!map->searchQueue.empty()) {
			UnitBase* unit = map->searchQueue.front();
			map->searchQueue.pop();
			unit->calculateNewPath();
		}
	}

	void callBuildings() {
		for (auto& building : buildings) {
			//building->update();
			building->draw();
		}
	}

	void debugText() {
		int fps = GetFPS();
		string fpsString = "FPS: " + to_string(fps);
		const char* fpsChar = fpsString.c_str();
		DrawText(fpsChar, 20, 1050, 24, GREEN);

		string TreeCount = "Trees: " + to_string(targetResourceCount->treeCount);
		DrawText(TreeCount.c_str(), 20, 1080, 24, PURPLE);
	}

	void update() {
		debugText();

		if (trainingUnits.size() < 3) {
			UnitToTrain nextUnit = getNextUnitToTrain();
			startTrainingUnits(nextUnit);
		}

		map->renderMap(SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE);
		callUnits();
		updateTrainingUnits();
		callBuildings();
	}
};
