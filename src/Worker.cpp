#include "Units.h"
#include "Worker.h"
#include "GameManager.h"

#include <cstdlib>
#include <iostream>

// set up fsm here
Worker::Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur) : UnitBase(_x, _y, _mp, _rt, _ur) {
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

	unitSpeed = 0.2f;
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
	//DrawRectangle(worker.goalPos.x, worker.goalPos.y, TILE_SIZE / 2, TILE_SIZE / 2, BLUE);

	if (worker.mapReference->fellTree(worker)) {
		worker.targetResourceTracker->treeCount++;
		int sameTileIdx = worker.treeTileTargetIdx;
		worker.mapReference->removeTreeByIndex(worker.treeTileTargetIdx, worker.treeTargetIdx);
		worker.treeTargetIdx = -1;
		worker.treeTileTargetIdx = -1;
		worker.currentPath.clear();
		worker.connectionIdx = 0;

		if (sameTileIdx != -1) {
			for (auto& entity : worker.mapReference->renderedTiles[sameTileIdx].occupyingEntities) {
				if (entity.entityType == eTree && !entity.reserved) {
					entity.reserved = true;
					worker.treeTargetIdx = entity.idx;
					worker.treeTileTargetIdx = sameTileIdx;
					worker.goalPos = worker.mapReference->renderedTiles[sameTileIdx].position + entity.tileOffset;
					return;
				}
			}
		}
		worker.goalPos = Vector2{ -1, -1 };
		return;
	}

	worker.chopTimer.updateTimer();

	if (worker.currentPath.empty()) {
		worker.goalPos = worker.mapReference->getNearestTreePos(worker);
		worker.calculateNewPath();
		if (worker.currentPath.empty()) return;
	}

	if (Vector2Distance(worker.pos, worker.targetPos) > 1.0f) {
		worker.moveUnitTowardsInternalGoal();
		return;
	}

	worker.connectionIdx++;
	if (worker.connectionIdx < worker.currentPath.size()) {
		worker.targetPos.x = (float)worker.currentPath[worker.connectionIdx].toNode.x;
		worker.targetPos.y = (float)worker.currentPath[worker.connectionIdx].toNode.y;
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