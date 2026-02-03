#pragma once

#include "raylib.h"
#include "raymath.h"
#include <FSM.h>

#include <iostream>
#include <vector>

#include <random>

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
	Vector2 pos; // current pos
	Vector2 targetPos; // sub-goal in a path
	Vector2 goalPos; // final goal in a path
	// All units should be equally big so it's hard coded here
	float size = 1.0f;
	float unitSpeed = 0.6f;
	const int TILE_SIZE = 10;
	int currentTileIdx = 0;
	int currentGoalTileIdx = 0;

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

	void moveFile();
	void testTile();
	void AwaitNewPath();
	int getcurrentCorrespondingTile(std::vector<Vector2>& pathToCheck, Vector2& _unitPos);
	virtual void renderUnit() = 0;
	virtual void commandUnit() = 0;

	virtual void calculateNewPath() {
		isAwaitingNewPath = false;
	};

	void moveUnitTowardsInternalGoal() {
		if (Vector2Distance(targetPos, pos) > 1.0f) {
			pos += Vector2Normalize(targetPos - pos) * unitSpeed;
		}

		float minDist = size * 2.0f;

		// avoid collision with other agents
		for (auto& unit : *_unitsReference) {
			if (unit && unit.get() != this) {
				Vector2 diff = pos - unit->pos;
				float distSq = Vector2LengthSqr(diff);

				if (distSq < minDist * minDist && distSq > 0.0f) {
					float dist = sqrtf(distSq);
					pos += (diff / dist) * (minDist - dist) * 0.5f;
				}
			}
		}
	}
};

// Scout can be created with 1 worker
struct Scout : UnitBase {
	Scout(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu) 
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {}
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, BLUE);
	}
	void calculateNewPath() override;
	void commandUnit();
};

struct CoalWorker : UnitBase {
	CoalWorker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu) 
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {}

	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, DARKGRAY);
	};
	void calculateNewPath() override;
};

struct Builder : UnitBase {
	Builder(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu) 
		: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {}

	Building* targetBuilding = nullptr;
	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, PINK);
	};
	void calculateNewPath() override;
};
