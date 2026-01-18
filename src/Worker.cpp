#include "Units.h"
#include "Worker.h"
#include "GameManager.h"
#include <cstdlib>

#include <iostream>

// set up fsm here
Worker::Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt) : UnitBase(_x, _y, _mp, _rt) {
	idleCheck->trueNode = targetIdeling;
	idleCheck->falseNode = nullptr;

	collectWoodCheck->trueNode = targetWoodcutting;
	collectWoodCheck->falseNode = nullptr;

	toIdle->decisionTreeRoot = idleCheck;
	toWoodcutting->decisionTreeRoot = collectWoodCheck;

	idelingState->transitions.push_back(toWoodcutting);
	collectingWoodState->transitions.push_back(toIdle);

	sm = new StateMachine<UnitBase>(collectingWoodState);
}

DecisionTreeNode<UnitBase>* IdleDecision::getBranch(UnitBase& agent) {
	if (agent.targetResourceTracker->treeCount >= 5) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<UnitBase>* CollectWoodDecision::getBranch(UnitBase& agent) {
	if (agent.targetResourceTracker->treeCount < 5) {
		return this->trueNode;
	}
	return this->falseNode;
}

// Run fsm through here
void Worker::commandUnit() {

	plans = sm->update(*this);

	for (auto& action : plans) {
		action->execute(*this);
	}

	//if (currentPath.size() == 0) {
	//	AwaitNewPath();
	//}
	//else {
	//	// Casually move toward targetPos
	//	if (Vector2Distance(pos, targetPos) > 10) {
	//		moveUnitTowardsInternalGoal();
	//	}
	//	// we hit our next goal
	//	if (Vector2Distance(pos, targetPos) < 5) {
	//		targetPos.x = (float)currentPath[connectionIdx].toNode.x;
	//		targetPos.y = (float)currentPath[connectionIdx].toNode.y;
	//		connectionIdx++;
	//	}
	//	// we reached the end of our path, so therefore reset and make new path
	//	if (connectionIdx >= currentPath.size() - 1) {
	//		currentPath.clear();
	//		connectionIdx = 0;
	//	}
	//}
}

void Worker::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;
	int randomNodeIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));
	currentTileIdx = getcurrentCorrespondingTile(mapReference->scoutedTiles->walkablePaths);

	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[randomNodeIdx],
		ref->walkablePathsNeighboors);

	UnitBase::calculateNewPath();
}

