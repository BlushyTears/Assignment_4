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
	float unitSpeed = 2.5f;
	const int TILE_SIZE = 10;
	int currentTileIdx = 0;
	bool isTraining = false;
	Map* mapReference = nullptr;
	int connectionIdx = 0;
	std::vector<Connection> currentPath;
	std::vector<Tile>* renderedTiles;
	ResourceTracker* resourceTracker = nullptr;

	UnitBase(int _x, int _y, Map* _mp, ResourceTracker* _rt);
	bool isAwaitingNewPath = false;

	void testTile();
	void AwaitNewPath();
	int getcurrentCorrespondingTile(std::vector<Vector2>& pathToCheck);
	virtual void renderUnit() = 0;
	virtual void moveUnit() = 0;

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
	void moveUnit();
};

struct Worker : UnitBase {
	Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt) : UnitBase(_x, _y, _mp, _rt) {}

	void moveUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, RED);
	};
	void calculateNewPath() override;
};

struct CoalBuilder : UnitBase {
	CoalBuilder(int _x, int _y, Map* _mp, ResourceTracker* _rt) : UnitBase(_x, _y, _mp, _rt) {}

	void moveUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, BLACK);
	};
	void calculateNewPath() override;
};

//// Idle state stuff
//template <typename T>
//struct IdleAction : Action<T> {
//	void execute(T& agent) override {
//		std::cout << "Unit is being idle" << std::endl;
//	}
//};
//
//template <typename T>
//struct IdleState : State<T> {
//	IdleAction<T> ideling;
//	std::vector<Transition<T>*> transitions;
//
//	std::vector<Action<T>*> getActions() override { return { &ideling }; };
//	std::vector<Transition<T>*> getTransitions() override { return transitions; };
//};
//
//template<typename T>
//struct TargetIdleState : TargetState<T> {
//	IdleState<T>* idleState;
//
//	TargetIdleState(IdleState<T>* s) : idleState(s) {}
//
//	std::vector<Action<T>*> getActions() override { return {}; }
//	State<T>* getTargetState() override { return idleState; }
//};
//
//template <typename T>
//struct IdleDecision : Decision<T> {
//	T testValue(T& agent) override { return agent; }
//	DecisionTreeNode<T>* getBranch(T& agent) override {
//		if (true) {
//			std::cout << "Decided to be idle" << std::endl;
//			return this->trueNode;
//		}
//		return this->falseNode;
//	}
//};