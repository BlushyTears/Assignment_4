#pragma once

#include <iostream>
#include <vector>
#include <string>
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
	Timer produceTimer;
	Timer buildTimer;
	bool isBuilt = false;
	bool isBuilding = false;
	bool materialsAvailable = false;
	int tileSize = 10;

	virtual void startBuildProcess() = 0;
	virtual void update() = 0;
	virtual void draw() = 0;
	virtual void debugText() = 0;
};

struct CoalMile : Building {
	ResourceTracker* resourceTracker = nullptr;
	int treeCount = 0;
	int coalCount = 0;
	int minTreesNeeded = 10;
	int costPerCoal = 2;
	bool isActive = false;

	CoalMile(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override;

	void startBuildProcess() override {
		if (treeCount >= minTreesNeeded) {
			treeCount -= minTreesNeeded;
		}
	}

	void putTreeInCoalMile(Worker& worker) {
		treeCount++;
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		else {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, DARKGRAY);
		}
		if (isActive) {
			DrawRectangle((int)this->pos.x + tileSize / 4, (int)this->pos.y + tileSize / 4, tileSize / 2, tileSize / 2, ORANGE);
		}
	}

	void debugText() override {
		std::string text = "Trees in coal mile: " + std::to_string(treeCount);
		DrawText(text.c_str(), 20, 1000, 16, PURPLE);

		std::string text2 = "Coal in coal mile: " + std::to_string(coalCount);
		DrawText(text2.c_str(), 20, 1020, 16, PURPLE);
	}
};

struct Smelter : Building {
	ResourceTracker* resourceTracker;
	int treeCount = 0;
	int ironCount = 0;
	int minTreesNeeded = 10;
	int costPerIron = 2;
	bool isActive = false;

	Smelter(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override;

	void startBuildProcess() override {
		if (treeCount >= minTreesNeeded) {
			treeCount -= minTreesNeeded;
		}
	}

	void putTreeInCoalMile(Worker& worker) {
		treeCount++;
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, DARKBROWN);
		}
		else {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, DARKBROWN);
		}
		if (isActive) {
			DrawRectangle((int)this->pos.x + tileSize / 4, (int)this->pos.y + tileSize / 4, tileSize / 2, tileSize / 2, ORANGE);
		}
	}

	void debugText() override {
		std::string text = "Trees in Smelter: " + std::to_string(treeCount);
		DrawText(text.c_str(), 220, 1000, 16, PURPLE);

		std::string text2 = "Iron bar in smelter: " + std::to_string(ironCount);
		DrawText(text2.c_str(), 220, 1020, 16, PURPLE);
	}
};

struct ArmSmith : Building {
	ResourceTracker* resourceTracker;
	int treeCount = 0;
	int coalCount = 0;
	int minTreesNeeded = 10;
	bool isActive = false;

	ArmSmith(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override {
		debugText();

		materialsAvailable = (treeCount >= minTreesNeeded);

		if (!isBuilt) return;

		if (isActive) {
			produceTimer.updateTimer();
			if (produceTimer.hasTimerEnded()) {
				coalCount++;
				isActive = false;
			}
		}
		else if (treeCount >= minTreesNeeded) {
			isActive = true;
			treeCount -= minTreesNeeded;
			produceTimer.setNewTimer(6);
		}
	}

	void startBuildProcess() override {
		if (treeCount >= minTreesNeeded) {
			treeCount -= minTreesNeeded;
		}
	}

	void putTreeInCoalMile(Worker& worker) {
		treeCount++;
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		else {
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, DARKGRAY);
		}
	}

	void debugText() override {
		std::string text = "Trees in armsmith: " + std::to_string(treeCount);
		DrawText(text.c_str(), 420, 1000, 16, PURPLE);
	}
};