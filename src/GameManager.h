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

struct ResourceTracker {
	int treeCount = 0;
	int coalCount = 0;
	int ironOreCount = 0;
	int ironBarCount = 0;
	int swordCount = 0;
};

// Purpose of Game manager: Manage everything game-related such as positions of things, , instantiations of other componenents etc
struct Game {
	stringstream mapData;
	// todo: turn these into smart pointers
	Map* map = nullptr; 
	//TaskManager* tm = nullptr;
	ResourceTracker* resourceTracker = nullptr;
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
		resourceTracker = new ResourceTracker();
		//tm = new TaskManager(resourceTracker, &units);

		for (int i = 0; i < _initialUnits; i++) {
			if (xCount % initialFormationColumns == 0) {
				xCount = 1;
				yCount++;
			}

			//units.push_back(std::make_unique<Scout>(
			//	baseXUnitSpawn + (spacing * xCount),
			//	baseYUnitSpawn + (spacing * yCount),
			//	map));
			//xCount++;

			if (i % 15 == 0) {
				units.push_back(std::make_unique<Scout>(
					baseXUnitSpawn + (spacing * xCount),
					baseYUnitSpawn + (spacing * yCount),
					map));
				xCount++;
			}
			else {
				units.push_back(std::make_unique<Worker>(
					baseXUnitSpawn + (spacing * xCount),
					baseYUnitSpawn + (spacing * yCount),
					map));
				xCount++;
			}

			units[i]->testTile();
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
		debugText();

		map->renderMap(SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE);
		callUnits();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
};
