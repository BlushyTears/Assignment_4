#pragma once

#include <chrono>
#include <thread>
#include <iostream>

#include <vector>
#include "Worker.h"
#include "raylib.h"

struct Game {
	std::vector<std::unique_ptr<UnitBase>> units;
	int initialFormationColumns = 11;
	int xCount = 1;
	int yCount = 1;
	int baseXSpawn = 80;
	int baseYSpawn = 200;
	int spacing = 10;

	Game(int _initialUnits) {
		for (int i = 0; i < _initialUnits; i++) {
			if (xCount % initialFormationColumns == 0) {
				xCount = 1;
				yCount++;
			}
			units.push_back(std::make_unique<Worker>(baseXSpawn + (spacing * xCount), baseYSpawn + (spacing * yCount)));
			xCount++;
		}
	}

	void renderUnits() {
		for (auto& unit : units) {
			unit->renderWorker();
		}
	}

	void update() {
		renderUnits();
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
};


