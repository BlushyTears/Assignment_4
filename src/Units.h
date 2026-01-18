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

struct UnitBase {
	Vector2 pos;
	Vector2 targetPos;
	// All units should be equally big so it's hard coded here
	int size = 3;
	float unitSpeed = 0.1f;
	const int TILE_SIZE = 10;
	int currentTileIdx = 0;
	bool isTraining = false;
	Map* mapReference = nullptr;
	int connectionIdx = 0;
	std::vector<Connection> currentPath;
	std::vector<Tile>* renderedTiles;
	ResourceTracker* targetResourceTracker = nullptr;

	UnitBase(int _x, int _y, Map* _mp, ResourceTracker* _rt);
	bool isAwaitingNewPath = false;

	void testTile();
	void AwaitNewPath();
	int getcurrentCorrespondingTile(std::vector<Vector2>& pathToCheck);
	virtual void renderUnit() = 0;
	virtual void commandUnit() = 0;

	virtual void calculateNewPath() {
		isAwaitingNewPath = false;
	};

	void moveUnitTowardsInternalGoal() {
		Vector2 dir = Vector2Normalize(targetPos - pos);
		pos += dir * unitSpeed;
	}
};

// Scout can be created with 1 worker
struct Scout : UnitBase {
	Scout(int _x, int _y, Map* _mp, ResourceTracker* _rt) : UnitBase(_x, _y, _mp, _rt) {}
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, BLUE);
	}
	void calculateNewPath() override;
	void commandUnit();
};

struct CoalWorker : UnitBase {
	CoalWorker(int _x, int _y, Map* _mp, ResourceTracker* _rt) : UnitBase(_x, _y, _mp, _rt) {}

	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, DARKGRAY);
	};
	void calculateNewPath() override;
};