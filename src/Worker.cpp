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
	if (worker.targetResourceTracker->treeCount >= 2500) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount < 100 && worker.mapReference->scoutedTiles->walkablePaths.size() > 100) {
		return this->trueNode;
	}
	return this->falseNode;
}

void IdleAction::execute(Worker& worker) {
	std::cout << "Unit is idle socuted path count: " << worker.mapReference->scoutedTiles->walkablePaths.size() << std::endl;
}

void CollectWoodAction::execute(Worker& worker) {
	std::cout << "Unit is collecting wood, wood count: " << worker.targetResourceTracker->treeCount << std::endl;

	if (worker.currentPath.size() == 0) {
		worker.AwaitNewPath();
	}
	else {
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
		if (worker.connectionIdx >= worker.currentPath.size() - 1) {
			worker.targetResourceTracker->treeCount++; // when we hit a position goal we "make" new tree
			worker.currentPath.clear();
			worker.connectionIdx = 0;
		}
	}
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

