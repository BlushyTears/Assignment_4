#pragma once

#include "raymath.h"
#include "raylib.h"
#include "resource_dir.h"

#include <iostream>

enum Behaviors {
	Seek,
	Wander
};

struct Agent;
struct Object;

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

	float rotationSmoothness;
	float orientation;
	Vector2 rotation;
	Vector2 forwardDirection;

	void updateBehavior();

	UnitBase(int _x, int _y) {
		pos.x = _x;
		pos.y = _y;
		targetPos = pos;
	}

	virtual void renderWorker() = 0;
	void moveUnit() {
		if (Vector2Distance(pos, targetPos) > 1) {
			Vector2 dir = targetPos - pos;
			pos += dir * unitSpeed;
		}
		else {
			targetPos.x = 400;
			targetPos.y = 400;
		}
	}
};

struct Worker : UnitBase {
	Worker(int _x, int _y) : UnitBase(_x, _y) {}
	void renderWorker() {
		DrawCircle(pos.x, pos.y, size, RED);
	}
};
