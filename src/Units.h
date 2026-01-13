#pragma once

#include "raylib.h"
#include "raymath.h"

#include <iostream>
#include <vector>

enum Behaviors {
	Seek,
	Wander
};

struct Agent;
struct Object;
struct Map;
struct Tile;

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
	float unitSpeed = 0.1f;
	const int TILE_SIZE = 10;

	Map* mapReference = nullptr;

	float rotationSmoothness;
	float orientation;
	Vector2 rotation;
	Vector2 forwardDirection;

	std::vector<Tile>* renderedTiles;

	UnitBase(int _x, int _y, Map* _mp);

	void testTile();

	virtual void renderWorker() = 0;
	void moveUnit() {
		if (Vector2Distance(pos, targetPos) > 1) {
			Vector2 dir = targetPos - pos;
			pos += dir * unitSpeed;
		}
		//else {
		//	targetPos.x = 400;
		//	targetPos.y = 400;
		//}
	}
};

// Scout can be created with 1 worker
struct Scout : UnitBase {
	Scout(int _x, int _y, Map* _mp) : UnitBase(_x, _y, _mp) {}
	void renderWorker() {
		DrawCircle(pos.x, pos.y, size, RED);
	}
};

struct Worker : UnitBase {
	Worker(int _x, int _y, Map* _mp) : UnitBase(_x, _y, _mp) {}
	void renderWorker() {
		DrawCircle(pos.x, pos.y, size, RED);
	}
};
