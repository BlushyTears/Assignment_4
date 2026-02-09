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
struct TrainingCamp;

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

// Collecting iron
struct CollectIronAction : Action<Worker> {
	// In here we set targetPos to nearest tree, go there and collect Iron
	void execute(Worker& worker) override;
};

struct CollectIronState : State<Worker> {
	CollectIronAction collectingIron;
	std::vector<Transition<Worker>*> transitions;

	std::vector<Action<Worker>*> getActions() override { return { &collectingIron }; };
	std::vector<Transition<Worker>*> getTransitions() override { return transitions; };
};

struct TargetCollectIronState : TargetState<Worker> {
	CollectIronState* collectIronState;

	TargetCollectIronState(CollectIronState* s) : collectIronState(s) {}

	std::vector<Action<Worker>*> getActions() override { return {}; }
	State<Worker>* getTargetState() override { return collectIronState; }
};

struct CollectIronDecision : Decision<Worker> {
	DecisionTreeNode<Worker>* getBranch(Worker& worker) override;
};

// Distribute resources between buildings
struct DistributinAction : Action<Worker> {
	void execute(Worker& worker) override;
};

struct DistributingState : State<Worker> {
	DistributinAction distributing;
	std::vector<Transition<Worker>*> transitions;

	std::vector<Action<Worker>*> getActions() override { return { &distributing }; };
	std::vector<Transition<Worker>*> getTransitions() override { return transitions; };
};

struct TargetDistributingState : TargetState<Worker> {
	DistributingState* distributingState;

	TargetDistributingState(DistributingState* s) : distributingState(s) {}

	std::vector<Action<Worker>*> getActions() override { return {}; }
	State<Worker>* getTargetState() override { return distributingState; }
};

struct DistributingDecision : Decision<Worker> {
	DecisionTreeNode<Worker>* getBranch(Worker& worker) override;
};

struct TrainUnitAction : Action<Worker> {
	void execute(Worker& worker) override;
};

struct TrainUnitState : State<Worker> {
	TrainUnitAction trainingUnit;
	std::vector<Transition<Worker>*> transitions;

	std::vector<Action<Worker>*> getActions() override { return { &trainingUnit }; };
	std::vector<Transition<Worker>*> getTransitions() override { return transitions; };
};

struct TargetTrainUnitState : TargetState<Worker> {
	TrainUnitState* trainingUnitState;

	TargetTrainUnitState(TrainUnitState* s) : trainingUnitState(s) {}

	std::vector<Action<Worker>*> getActions() override { return {}; }
	State<Worker>* getTargetState() override { return trainingUnitState; }
};

struct TrainUnitDecision : Decision<Worker> {
	DecisionTreeNode<Worker>* getBranch(Worker& worker) override;
};

struct Worker : UnitBase {
	CollectWoodState* collectingWoodState;
	CollectIronState* collectingIronState;
	DistributingState* distributingState;
	TrainUnitState* trainingState;

	TargetCollectWoodState* targetWoodcutting;
	TargetCollectIronState* targetIronCollecting;
	TargetDistributingState* targetDistribution;
	TargetTrainUnitState* targetTraining;

	CollectWoodDecision* collectWoodCheck;
	CollectIronDecision* collectIronCheck;
	DistributingDecision* distributeCheck;
	TrainUnitDecision* trainingCheck;

	DecisionTreeTransition<Worker>* toWoodcutting;
	DecisionTreeTransition<Worker>* toIronCollecting;
	DecisionTreeTransition<Worker>* toDistributing;
	DecisionTreeTransition<Worker>* toTraining;

	StateMachine<Worker>* sm;
	std::vector<Action<Worker>*> plans;

	Timer chopTimer;

	int treeTileTargetIdx = -1;
	int treeTargetIdx = -1;
	bool isChoppingWood = false;
	bool isCarryingWood = false;
	bool isCarryingIron = false;

	bool isCarryingCoal = false;
	bool isCarryingIronBar = false;
	bool isCarryingIronArrow = false;
	bool isCarryingIronSword = false;

	bool isDevotedToBeSoldier = false;

	bool isReadyToTrain = false;
	TrainingCamp* trainingCamp = nullptr;

	Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu);
	~Worker();

	void commandUnit() override;
	void renderUnit() {
		if (isCarryingWood) {
			DrawCircle(pos.x + size, pos.y + size, size, GREEN);
		}
		if (isCarryingIron) {
			DrawCircle(pos.x + size, pos.y + size, size, DARKGRAY);
		}
		if (isCarryingCoal) {
			DrawCircle(pos.x + size, pos.y - size, size, BLACK);
		}
		if (isCarryingIronBar) {
			DrawRectangle(pos.x + size, pos.y, size * 2, size, LIGHTGRAY);
		}
		if (isCarryingIronArrow) {
			DrawLine(pos.x + size, pos.y, pos.x + size * 2, pos.y + size, DARKBROWN);
		}
		if (isCarryingIronSword) {
			DrawLine(pos.x + size, pos.y, pos.x + size * 2, pos.y + size, WHITE);
		}

		DrawCircle(pos.x, pos.y, size, RED);
	};
	void calculateNewPath() override;
};
