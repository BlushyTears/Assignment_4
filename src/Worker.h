#pragma once

#include <iostream>
#include <vector>
#include <random>

#include "raylib.h"
#include "raymath.h"
#include <FSM.h>
#include "Units.h"

// Since workers are so much more complex, they need their own header and FSM

struct ResourceTracker;
struct Worker;

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

	Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt);

	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, RED);
	};
	void calculateNewPath() override;
};
