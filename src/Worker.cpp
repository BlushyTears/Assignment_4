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

	sm = new StateMachine<Worker>(collectingWoodState);

	unitSpeed = 0.5f;
}

DecisionTreeNode<Worker>* IdleDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount > 10000) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount < 10000) {
		return this->trueNode;
	}
	return this->falseNode;
}

void IdleAction::execute(Worker& worker) {
	std::cout << "Unit is idle tree count: " << worker.targetResourceTracker->treeCount << std::endl;
}

void CollectWoodAction::execute(Worker& worker) {
	while (worker.currentPath.empty() || worker.connectionIdx >= (int)worker.currentPath.size()) {
		Vector2 newTreePos = worker.mapReference->getNearestTreePos(worker);

		if (Vector2Distance(worker.pos, newTreePos) < 0.1f) {
			return;
		}

		worker.goalPos = newTreePos;
		worker.calculateNewPath();

		if (worker.currentPath.empty()) {
			worker.mapReference->removeTreeAtPos(newTreePos);
			continue;
		}
		else {
			worker.connectionIdx = 0;
			worker.targetPos.x = worker.currentPath[0].toNode.x;
			worker.targetPos.y = worker.currentPath[0].toNode.y;
			break;
		}
	}

	if (Vector2Distance(worker.pos, worker.targetPos) > 1.0f) {
		worker.moveUnitTowardsInternalGoal();
		return;
	}

	worker.connectionIdx++;
	if (worker.connectionIdx < (int)worker.currentPath.size()) {
		worker.targetPos.x = worker.currentPath[worker.connectionIdx].toNode.x;
		worker.targetPos.y = worker.currentPath[worker.connectionIdx].toNode.y;
		return;
	}

	if (worker.mapReference->fellTree(worker)) {
		worker.targetResourceTracker->treeCount++;
		worker.mapReference->removeTreeAtPos(worker.goalPos);
	}

	worker.currentPath.clear();
	worker.connectionIdx = 0;
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

	int randomNodeIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));

	this->currentTileIdx = getcurrentCorrespondingTile(mapReference->scoutedTiles->walkablePaths, this->pos);
	this->currentGoalTileIdx = getcurrentCorrespondingTile(mapReference->scoutedTiles->walkablePaths, this->goalPos);
	currentPath = ref->AStar(
					ref->walkablePaths[this->currentTileIdx],
					ref->walkablePaths[this->currentGoalTileIdx],
					ref->walkablePathsNeighboors);

	if (currentPath.size() > 0) {
		this->connectionIdx = 0;
		this->targetPos.x = (float)this->currentPath[this->connectionIdx].toNode.x;
		this->targetPos.y = (float)this->currentPath[this->connectionIdx].toNode.y;
	}
	
	UnitBase::calculateNewPath();
}