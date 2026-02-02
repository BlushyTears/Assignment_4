#pragma once

#include <iostream>
#include <vector>
#include "Units.h"

#include "raylib.h"
#include "GameManager.h"
#include "MapManager.h"
#include "Timer.h"

#include "Worker.h"

struct ResourceTracker;
struct Timer;
struct Worker;

struct Building {
	Vector2 pos;
	Timer timer;

	virtual void update() = 0;
	virtual void draw() = 0;
};

struct CoalMile : Building {
	ResourceTracker* resourceTracker;
	int tileSize = 0;
	int treeCount = 0;
	int coalCount = 0;
	int minTreesNeeded = 10;
	bool isBuilt = false;
	bool isActive = false;

	CoalMile(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override {
		debugText();

		if (!timer.hasTimerEnded() && isActive) {
			timer.updateTimer();
			return;
		}
		else if (timer.hasTimerEnded()) {
			// if we aren't smelting (This should be guaranteed to run once)
			if (!isActive && treeCount >= minTreesNeeded) {
				isActive = true;
				timer.setNewTimer(6);
			}
			// if we are done smelting our current batch
			else if(isActive) {
				treeCount -= 10;
				coalCount++;
				treeCount--;
				isActive = false;
			}
		}
	}

	void putTreeInCoalMile(Worker& worker) {
		treeCount++;
		std::cout << "Worker put in tree in coal mile";
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines(this->pos.x, this->pos.y, tileSize, tileSize, BLACK);
		}
		else {
			DrawRectangle(this->pos.x, this->pos.y, tileSize, tileSize, BLACK);
			DrawRectangle(this->pos.x, this->pos.y, tileSize, tileSize, DARKGRAY);
		}
	}

	void debugText() {
		string TreeCount = "Trees in coal mile: " + to_string(treeCount);
		DrawText(TreeCount.c_str(), 20, 1080, 24, PURPLE);
	}
};
