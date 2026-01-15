#pragma once

#include "raylib.h"
#include "raymath.h"

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

struct Agent;
struct Object;
struct Map;
struct Tile;
struct Connection;

struct MovementBehavior {
	virtual ~MovementBehavior() = default;
	virtual void execute(Agent& agent, Object* player) = 0;
};

struct SeekBehavior : MovementBehavior {
	virtual Vector2 getTargetDirection(Agent& agent, Object* player);
	void execute(Agent& agent, Object* player) override;
};

struct WanderBehavior : MovementBehavior {
	float wanderOrientation = 1.0f;
	float wanderOffset = 250;
	float wanderRadius = 35;
	float wanderRate = 1.0f;
	void execute(Agent& agent, Object* player) override;
};

struct UnitBase {
	Vector2 pos;
	Vector2 targetPos;
	// All units should be equally big so it's hard coded here
	int size = 3;
	float unitSpeed = 2.5f;
	const int TILE_SIZE = 10;
	int currentTileIdx = 0;

	Map* mapReference = nullptr;
	int connectionIdx = 0;
	std::vector<Connection> currentPath;

	float rotationSmoothness;
	float orientation;
	Vector2 rotation;
	Vector2 forwardDirection;

	std::vector<Tile>* renderedTiles;

	UnitBase(int _x, int _y, Map* _mp);

	void testTile();
	int getcurrentCorrespondingTile(std::vector<Vector2>& pathToCheck);
	virtual void renderUnit() = 0;
	virtual void moveUnit() = 0;

	void moveUnitTowardsInternalGoal() {
		Vector2 dir = Vector2Normalize(targetPos - pos);
		pos += dir * unitSpeed;
	}
};

// Scout can be created with 1 worker
struct Scout : UnitBase {
	Scout(int _x, int _y, Map* _mp) : UnitBase(_x, _y, _mp) {}
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, BLUE);
	}

	void moveUnit();
};

struct Worker : UnitBase {
	Worker(int _x, int _y, Map* _mp) : UnitBase(_x, _y, _mp) {}

	void moveUnit();
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, RED);
	}
};
