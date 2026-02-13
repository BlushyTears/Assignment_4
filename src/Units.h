#pragma once

#include "raylib.h"
#include "raymath.h"
#include <FSM.h>

#include <iostream>
#include <vector>
#include <random>

#include <FileRead.h>

inline int getRandomNumber(int min, int max) {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(min, max);
	return dist6(rng);
}

enum Behaviors {
	Seek,
	Wander
};

struct Map;
struct Tile;
struct Connection;
struct ResourceTracker;
struct Building;

struct UnitBase {
	Vector2 pos;
	Vector2 targetPos;
	Vector2 goalPos;
	float size = 0.8f;
	float unitSpeed;
	const int TILE_SIZE = 10;
	int currentTileIdx = 0;
	int currentGoalTileIdx = 0;
	int tileCountSinceComputeNeighboors = 0;

	bool isTraining = false;
	Map* mapReference = nullptr;
	int connectionIdx = 0;
	std::vector<Connection> currentPath;
	std::vector<Tile>* renderedTiles;
	ResourceTracker* targetResourceTracker = nullptr;
	std::vector<std::unique_ptr<UnitBase>>* _unitsReference = nullptr;
	std::vector<Building*>& buildings;

	UnitBase(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu);
	bool isAwaitingNewPath = false;

	void testTile();
	void AwaitNewPath();
	int getcurrentCorrespondingTile(std::vector<Vector2>& pathToCheck, Vector2& _unitPos);
	void terrainControl();
	virtual void renderUnit() = 0;
	virtual void commandUnit() = 0;

	virtual void calculateNewPath() {
		isAwaitingNewPath = false;
	};

	void moveUnitTowardsInternalGoal() {
		terrainControl(); // check if it's swamp or grass before moving unit
		Vector2 steering = { 0, 0 };

		if (goalPos.x != -1 && Vector2Distance(pos, targetPos) > 0.5f) {
			steering += Vector2Normalize(targetPos - pos) * unitSpeed * GetFrameTime() * resourceParameters.timeControl;
		}

		pos += steering;
	}
};

struct Scout : UnitBase {
	Scout(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu)
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {
	}
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, BLUE);
	}
	void calculateNewPath() override;
	void commandUnit();
};

struct Builder : UnitBase {
	Builder(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu)
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {
	}

	Building* targetBuilding = nullptr;
	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, PINK);
	};
	void calculateNewPath() override;

};

struct CoalWorker : UnitBase {
	CoalWorker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu)
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {
	}

	Building* targetBuilding = nullptr;
	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, DARKGRAY);
	};
	void calculateNewPath() override;
};

struct SmelterWorker : UnitBase {
	SmelterWorker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu)
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {
	}

	Building* targetBuilding = nullptr;
	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, GRAY);
	};
	void calculateNewPath() override;
};

struct ArmSmithWorker : UnitBase {
	ArmSmithWorker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu)
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {
	}

	Building* targetBuilding = nullptr;
	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, BLACK);
	};
	void calculateNewPath() override;
};

struct Soldier : UnitBase {
	Soldier(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu)
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {
	}

	Building* targetBuilding = nullptr;
	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, GOLD);
	};
	void calculateNewPath() override;
};