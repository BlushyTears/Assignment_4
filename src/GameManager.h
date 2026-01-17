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
	int solderCount = 0;
};

// Goal: Make 3 scouts, then find some trees to make a coal mile and then build a coal mile
// We need to convert some workers into scouts. 

// Purpose of Game manager: Manage everything game-related such as positions of things, , instantiations of other componenents etc
struct Game {
	stringstream mapData;
	// todo: turn these into smart pointers
	Map* map = nullptr; 
	//TaskManager* tm = nullptr;
	ResourceTracker* resourceTrackerInternal = nullptr;
	ResourceTracker* resourceTrackerDisplay = nullptr;
	std::vector<std::unique_ptr<UnitBase>> units;

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
				map));
			xCount++;

			units[i]->testTile();
		}
	}

	void assignTasks() {
		if (resourceTrackerInternal->scoutCount <= 3) {
			for (auto& unit : units) {
				Worker* worker = dynamic_cast<Worker*>(unit.get());

				if (worker) {
					Vector2 tempPos = worker->pos;

					unit = std::make_unique<Scout>(tempPos.x, tempPos.y, map);
					resourceTrackerInternal->scoutCount++;
					resourceTrackerInternal->workerCount--;
					break;
				}
			}
		}
	}

	void callUnits() {
		for (auto& unit : units) {
			unit->renderUnit();
			unit->moveUnit();
			unit->moveUnitTowardsInternalGoal();
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
		assignTasks();
		debugText();

		map->renderMap(SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE);
		callUnits();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
};
