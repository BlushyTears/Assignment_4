#pragma once

#include <iostream>
#include <vector>
#include <random>

#include "raylib.h"
#include "raymath.h"
#include <FSM.h>
#include "Units.h"

#include "Timer.h"
#include "Building.h"

// Note: Since workers are so much more complex, they need their own header and FSM

struct ResourceTracker;
struct Worker;
struct Timer;
struct Building;

// Idle state stuff
struct IdleAction : Action<Worker> {
	void execute(Worker& worker) override;
};

struct IdleState : State<Worker> {
	IdleAction ideling;
	std::vector<Transition<Worker>*> transitions;

	std::vector<Action<Worker>*> getActions() override { return { &ideling }; };
	std::vector<Transition<Worker>*> getTransitions() override { return transitions; };
};

struct TargetIdleState : TargetState<Worker> {
	IdleState* idleState;

	TargetIdleState(IdleState* s) : idleState(s) {}

	std::vector<Action<Worker>*> getActions() override { return {}; }
	State<Worker>* getTargetState() override { return idleState; }
};

struct IdleDecision : Decision<Worker> {
	DecisionTreeNode<Worker>* getBranch(Worker& worker) override;
};

// Farming trees
struct CollectWoodAction : Action<Worker> {
	// In here we set targetPos to nearest tree, go there and collect wood
	void execute(Worker& worker) override;
};

struct CollectWoodState : State<Worker> {
	CollectWoodAction collectingWood;
	std::vector<Transition<Worker>*> transitions;

	std::vector<Action<Worker>*> getActions() override { return { &collectingWood }; };
	std::vector<Transition<Worker>*> getTransitions() override { return transitions; };
};

struct TargetCollectWoodState : TargetState<Worker> {
	CollectWoodState* collectWoodState;

	TargetCollectWoodState(CollectWoodState* s) : collectWoodState(s) {}

	std::vector<Action<Worker>*> getActions() override { return {}; }
	State<Worker>* getTargetState() override { return collectWoodState; }
};

struct CollectWoodDecision : Decision<Worker> {
	DecisionTreeNode<Worker>* getBranch(Worker& worker) override;
};

struct Worker : UnitBase {
	IdleState* idelingState;
	CollectWoodState* collectingWoodState;

	TargetIdleState* targetIdeling;
	TargetCollectWoodState* targetWoodcutting;

	IdleDecision* idleCheck;
	CollectWoodDecision* collectWoodCheck;

	DecisionTreeTransition<Worker>* toIdle;
	DecisionTreeTransition<Worker>* toWoodcutting;

	StateMachine<Worker>* sm;
	std::vector<Action<Worker>*> plans;

	std::vector<Building*>& buildings;

	Timer chopTimer;

	int treeTileTargetIdx = -1; // which tree on some particular tile do we target (1-5)
	int treeTargetIdx = -1; // which tree on some particular tile do we target (1-5)
	bool isChoppingWood = false;
	bool isCarryingWood = false;

	//MapIndex treeToChop;
	//bool hasWoodLogs = false;

	Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu);

	void commandUnit() override;
	void renderUnit() {
		if (isCarryingWood) {
			DrawCircle(pos.x + size, pos.y + size, size, BROWN);
		}

		DrawCircle(pos.x, pos.y, size, RED);
	};
	void calculateNewPath() override;
};