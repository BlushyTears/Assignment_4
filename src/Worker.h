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
struct UnitBase;

// Idle state stuff
struct IdleAction : Action<UnitBase> {
	void execute(UnitBase& agent) override;
};

struct IdleState : State<UnitBase> {
	IdleAction ideling;
	std::vector<Transition<UnitBase>*> transitions;

	std::vector<Action<UnitBase>*> getActions() override { return { &ideling }; };
	std::vector<Transition<UnitBase>*> getTransitions() override { return transitions; };
};

struct TargetIdleState : TargetState<UnitBase> {
	IdleState* idleState;

	TargetIdleState(IdleState* s) : idleState(s) {}

	std::vector<Action<UnitBase>*> getActions() override { return {}; }
	State<UnitBase>* getTargetState() override { return idleState; }
};

struct IdleDecision : Decision<UnitBase> {
	DecisionTreeNode<UnitBase>* getBranch(UnitBase& agent) override;
};

// Farming trees
struct CollectWoodAction : Action<UnitBase> {
	// In here we set targetPos to nearest tree, go there and collect wood
	void execute(UnitBase& agent) override;
};

struct CollectWoodState : State<UnitBase> {
	CollectWoodAction collectingWood;
	std::vector<Transition<UnitBase>*> transitions;

	std::vector<Action<UnitBase>*> getActions() override { return { &collectingWood }; };
	std::vector<Transition<UnitBase>*> getTransitions() override { return transitions; };
};

struct TargetCollectWoodState : TargetState<UnitBase> {
	CollectWoodState* collectWoodState;

	TargetCollectWoodState(CollectWoodState* s) : collectWoodState(s) {}

	std::vector<Action<UnitBase>*> getActions() override { return {}; }
	State<UnitBase>* getTargetState() override { return collectWoodState; }
};

struct CollectWoodDecision : Decision<UnitBase> {
	DecisionTreeNode<UnitBase>* getBranch(UnitBase& agent) override;
};

struct Worker : UnitBase {
	IdleState* idelingState;
	CollectWoodState* collectingWoodState;

	TargetIdleState* targetIdeling;
	TargetCollectWoodState* targetWoodcutting;

	IdleDecision* idleCheck;
	CollectWoodDecision* collectWoodCheck;

	DecisionTreeTransition<UnitBase>* toIdle;
	DecisionTreeTransition<UnitBase>* toWoodcutting;

	StateMachine<UnitBase>* sm;
	std::vector<Action<UnitBase>*> plans;

	Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt);

	void commandUnit() override;
	void renderUnit() {
		DrawCircle(pos.x, pos.y, size, RED);
	};
	void calculateNewPath() override;
};
