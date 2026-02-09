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
	int treeCount = 10;
	int minTreesNeeded = 10;
	bool isBuilt = true;
	bool isBuilding = false;
	int tileSize = 10;

	virtual void startBuildProcess() = 0;
	virtual void update() = 0;
	virtual void draw() = 0;
	virtual void debugText() = 0;
};

struct CoalMile : Building {
	ResourceTracker* resourceTracker = nullptr;
	int coalCount = 0;
	int costPerCoal = 2;
	bool isActive = false;

	CoalMile(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override;

	void startBuildProcess() override {
		if (!isBuilt && !isBuilding && treeCount >= minTreesNeeded) {
			treeCount -= minTreesNeeded;
			isBuilding = true;
		}
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		else {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, WHITE);
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
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
	ResourceTracker* resourceTracker = nullptr;
	int ironOreCount = 0;
	int coalCostPerIronBar = 2;
	int ironBarCount = 0;
	int ironArrowCount = 0;
	int ironArrowsGoal = 3;
	bool producingArrows = false;

	int coalCount = 0;
	bool isActive = false;

	Smelter(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override;

	void startBuildProcess() override {
		if (!isBuilt && !isBuilding && treeCount >= minTreesNeeded) {
			treeCount -= minTreesNeeded;
			isBuilding = true;
		}
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		else {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, LIGHTGRAY);
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		if (isActive) {
			DrawRectangle((int)this->pos.x + tileSize / 4, (int)this->pos.y + tileSize / 4, tileSize / 2, tileSize / 2, RED);
		}
	}

	void debugText() override {
		std::string text1 = "Trees in smelter: " + std::to_string(treeCount);
		DrawText(text1.c_str(), 220, 1000, 16, PURPLE);

		std::string text2 = "Coal in smelter: " + std::to_string(coalCount);
		DrawText(text2.c_str(), 220, 1020, 16, PURPLE);

		std::string text3 = "Iron ores in smelter: " + std::to_string(ironOreCount);
		DrawText(text3.c_str(), 220, 1040, 16, PURPLE);

		std::string text4 = "Iron bars in smelter: " + std::to_string(ironBarCount);
		DrawText(text4.c_str(), 220, 1060, 16, PURPLE);

		std::string text5 = "Iron arrows in smelter: " + std::to_string(ironArrowCount);
		DrawText(text5.c_str(), 220, 1080, 16, PURPLE);

	}
};

struct ArmSmith : Building {
	ResourceTracker* resourceTracker = nullptr;
	int coalCount = 0;
	int coalNeeded = 2;
	int ironArrowCount = 0;
	int ironArrowsNeeded = 3;
	int ironBarCount = 0;
	int ironBarNeeded = 1;

	int ironSwordCount = 0;
	bool isActive = false;

	ArmSmith(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override;

	void startBuildProcess() override {
		if (!isBuilt && !isBuilding && treeCount >= minTreesNeeded && ironArrowCount >= ironArrowsNeeded) {
			treeCount -= minTreesNeeded;
			ironArrowCount -= ironArrowsNeeded;
			isBuilding = true;
		}
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		else {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, DARKGRAY);
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		if (isActive) {
			DrawRectangle((int)this->pos.x + tileSize / 4, (int)this->pos.y + tileSize / 4, tileSize / 2, tileSize / 2, PINK);
		}
	}

	void debugText() override {
		std::string text2 = "Trees in arm smith: " + std::to_string(treeCount);
		DrawText(text2.c_str(), 420, 1000, 16, PURPLE);

		std::string text4 = "Iron bars in armsmith: " + std::to_string(ironBarCount);
		DrawText(text4.c_str(), 420, 1020, 16, PURPLE);

		std::string text = "Iron Arrows in armsmith: " + std::to_string(ironArrowCount);
		DrawText(text.c_str(), 420, 1040, 16, PURPLE);

		std::string text3 = "Iron sword in armsmith: " + std::to_string(ironSwordCount);
		DrawText(text3.c_str(), 420, 1060, 16, PURPLE);

		std::string text5 = "Coal count in arm smith: " + std::to_string(coalCount);
		DrawText(text5.c_str(), 420, 1080, 16, PURPLE);
	}
};

struct TrainingCamp : Building {
	ResourceTracker* resourceTracker = nullptr;

	bool isWorkerAvailable = false;
	bool isActive = false;
	int swordCount = 00;
	int swordsNeeded = 1;

	TrainingCamp(Vector2 _pos, ResourceTracker* _rt, int _tileSize) {
		pos = _pos;
		resourceTracker = _rt;
		tileSize = _tileSize;
	}

	void update() override;

	void startBuildProcess() override {
		if (!isBuilt && !isBuilding && treeCount >= minTreesNeeded) {
			treeCount -= minTreesNeeded;
			isBuilding = true;
		}
	}

	void draw() override {
		if (!isBuilt) {
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		else {
			DrawRectangle((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, DARKGREEN);
			DrawRectangleLines((int)this->pos.x, (int)this->pos.y, tileSize, tileSize, BLACK);
		}
		if (isActive) {
			DrawRectangle((int)this->pos.x + tileSize / 4, (int)this->pos.y + tileSize / 4, tileSize / 2, tileSize / 2, BLACK);
		}
	}

	void debugText() override {
		std::string text1 = "Trees in training camp: " + std::to_string(treeCount);
		DrawText(text1.c_str(), 650, 1000, 16, PURPLE);

		std::string text2 = "Swords in training camp: " + std::to_string(swordCount);
		DrawText(text2.c_str(), 650, 1020, 16, PURPLE);
	}
};