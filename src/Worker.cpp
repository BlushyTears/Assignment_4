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

	toIdle = new DecisionTreeTransition<UnitBase>();
	toWoodcutting = new DecisionTreeTransition<UnitBase>();

	idleCheck->trueNode = targetIdeling;
	idleCheck->falseNode = nullptr;
	collectWoodCheck->trueNode = targetWoodcutting;
	collectWoodCheck->falseNode = nullptr;

	toIdle->decisionTreeRoot = idleCheck;
	toWoodcutting->decisionTreeRoot = collectWoodCheck;

	idelingState->transitions.push_back(toWoodcutting);
	collectingWoodState->transitions.push_back(toIdle);

	sm = new StateMachine<UnitBase>(idelingState);
}

DecisionTreeNode<UnitBase>* IdleDecision::getBranch(UnitBase& agent) {
	// Only start chopping trees once the map is a certain size (testing reasons)
	if (agent.targetResourceTracker->treeCount >= 2500) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<UnitBase>* CollectWoodDecision::getBranch(UnitBase& agent) {
	if (agent.targetResourceTracker->treeCount < 100 && agent.mapReference->scoutedTiles->walkablePaths.size() > 100) {
		return this->trueNode;
	}
	return this->falseNode;
}

void IdleAction::execute(UnitBase& agent) {
	std::cout << "Unit is idle socuted path count: " << agent.mapReference->scoutedTiles->walkablePaths.size() << std::endl;
}

void CollectWoodAction::execute(UnitBase& agent) {
	std::cout << "Unit is collecting wood, wood count: " << agent.targetResourceTracker->treeCount << std::endl;

	if (agent.currentPath.size() == 0) {
		agent.AwaitNewPath();
	}
	else {
		// Casually move toward targetPos
		if (Vector2Distance(agent.pos, agent.targetPos) > 10) {
			agent.moveUnitTowardsInternalGoal();
		}
		// we hit our next goal
		if (Vector2Distance(agent.pos, agent.targetPos) < 5) {
			agent.targetPos.x = (float)agent.currentPath[agent.connectionIdx].toNode.x;
			agent.targetPos.y = (float)agent.currentPath[agent.connectionIdx].toNode.y;
			agent.connectionIdx++;
		}
		// we reached the end of our path, so therefore reset and make new path
		if (agent.connectionIdx >= agent.currentPath.size() - 1) {
			agent.targetResourceTracker->treeCount++; // when we hit a position goal we make new tree
			agent.currentPath.clear();
			agent.connectionIdx = 0;
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

