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

constexpr int SCREEN_WIDTH = 1000;
constexpr int SCREEN_HEIGHT = 1200;
constexpr int TILE_SIZE = 10;

struct Game {
	stringstream mapData;
	Map* map = nullptr; // todo: turn into smart pointer

	std::vector<std::unique_ptr<UnitBase>> units;
	int initialFormationColumns = 11;
	int xCount = 1;
	int yCount = 1;
	int baseXSpawn = 80;
	int baseYSpawn = 200;
	int spacing = 10;

	Game(int _initialUnits) {
		mapData = transcribeData("..//mapData.txt");
		map = new Map(mapData, SCREEN_WIDTH, TILE_SIZE);

		for (int i = 0; i < _initialUnits; i++) {
			if (xCount % initialFormationColumns == 0) {
				xCount = 1;
				yCount++;
			}

			if (i % 2 == 0) {
				units.push_back(std::make_unique<Worker>(
					baseXSpawn + (spacing * xCount),
					baseYSpawn + (spacing * yCount),
					map));
				xCount++;
			}
			else {
				units.push_back(std::make_unique<Scout>(
					baseXSpawn + (spacing * xCount),
					baseYSpawn + (spacing * yCount),
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
	}

	void update() {
		map->renderMap(SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE);
		callUnits();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
};