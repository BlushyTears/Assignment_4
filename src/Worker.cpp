#include "Units.h"
#include "Worker.h"
#include "GameManager.h"

#include <cstdlib>
#include <iostream>

// set up fsm here
Worker::Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt) : UnitBase(_x, _y, _mp, _rt) {
	idelingState = new IdleState();
	collectingWoodState = new CollectWoodState();

	targetIdeling = new TargetIdleState(idelingState);
	targetWoodcutting = new TargetCollectWoodState(collectingWoodState);

	idleCheck = new IdleDecision();
	collectWoodCheck = new CollectWoodDecision();

	toIdle = new DecisionTreeTransition<Worker>();
	toWoodcutting = new DecisionTreeTransition<Worker>();

	idleCheck->trueNode = targetIdeling;
	idleCheck->falseNode = nullptr;
	collectWoodCheck->trueNode = targetWoodcutting;
	collectWoodCheck->falseNode = nullptr;

	toIdle->decisionTreeRoot = idleCheck;
	toWoodcutting->decisionTreeRoot = collectWoodCheck;

	idelingState->transitions.push_back(toWoodcutting);
	collectingWoodState->transitions.push_back(toIdle);

	sm = new StateMachine<Worker>(idelingState);
}

DecisionTreeNode<Worker>* IdleDecision::getBranch(Worker& worker) {
	// Only start chopping trees once the map is a certain size (testing reasons)
	if (worker.targetResourceTracker->treeCount >= 50) {
		worker.shouldWander = true;
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount < 200 && worker.goalIdx == 0) {
		worker.goalIdx = worker.mapReference->getNearestTreeIdx(worker);
		worker.shouldWander = false;
		return this->trueNode;
	}
	return this->falseNode;
}

void IdleAction::execute(Worker& worker) {
	//std::cout << "Unit is idle socuted path count: " << worker.mapReference->scoutedTiles->walkablePaths.size() << std::endl;
}

void CollectWoodAction::execute(Worker& worker) {
	//std::cout << "Unit is cutting wood. goal index: " << worker.goalIdx << std::endl;
	if (worker.goalIdx == -1) {
		worker.goalIdx = worker.mapReference->getNearestTreeIdx(worker);
	}
	else {
		if (worker.currentPath.size() == 0) {
			worker.AwaitNewPath();
		}
		else if (worker.currentPath.size() > 0) {
			// Casually move toward targetPos
			if (Vector2Distance(worker.pos, worker.targetPos) > 10) {
				worker.moveUnitTowardsInternalGoal();
			}
			// we hit our next goal
			if (Vector2Distance(worker.pos, worker.targetPos) < 5) {
				worker.targetPos.x = (float)worker.currentPath[worker.connectionIdx].toNode.x;
				worker.targetPos.y = (float)worker.currentPath[worker.connectionIdx].toNode.y;
				worker.connectionIdx++;
			}
			// we reached the end of our path, so therefore reset and make new path
			if (worker.connectionIdx >= worker.currentPath.size()) {
				//worker.

				worker.mapReference->deleteTree(worker.goalIdx);
				worker.targetResourceTracker->treeCount++; // when we hit a position goal we "make" new tree
				worker.goalIdx = 0; // reset our goal
				worker.currentPath.clear();
				worker.connectionIdx = 0;
			}
		}
	}
}

// Run fsm through here
void Worker::commandUnit() {
	plans = sm->update(*this);

	for (auto& action : plans) {
		action->execute(*this);
	}
}

void Worker::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;
	int targetIdx = 0;

	if (shouldWander) {
		targetIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));
	}
	else {
		targetIdx = goalIdx;
	}

	currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths);
	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[targetIdx], // position of targetIndex in full map. Vector2
		ref->walkablePathsNeighboors);

	if(currentPath.size() > 0) {}
	//std::cout << "path" << std::endl;
	UnitBase::calculateNewPath();
}
