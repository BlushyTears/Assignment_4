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

	// worker goal idx is more intricate so it needs to start at -1
}

DecisionTreeNode<Worker>* IdleDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount > 1000) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount < 1000) {
		return this->trueNode;
	}
	return this->falseNode;
}

void IdleAction::execute(Worker& worker) {
	std::cout << "Unit is idle tree count: " << worker.targetResourceTracker->treeCount << std::endl;
}

void CollectWoodAction::execute(Worker& worker) {
	//std::cout << "Unit is cutting wood. goal index: " << worker.goalIdx << std::endl;
	float distanceToTargetPos = Vector2Distance(worker.pos, worker.targetPos);

	DrawRectangle(worker.goalPos.x, worker.goalPos.y, 10, 10, PURPLE);
	string currenPathSize = to_string(worker.currentPath.size()) + " Is path size";
	const char* c = currenPathSize.c_str();
	DrawText(c, 500, 1100, 24, GREEN);

	//std::cout << worker.currentPath.size() << " Is the size for worker X" << std::endl;

	if (Vector2Distance(worker.pos, worker.goalPos) < 1) {
		worker.goalPos = worker.mapReference->getNearestTreePos(worker);
		return;
	}
	else {
		if (worker.currentPath.size() == 0) {
			worker.AwaitNewPath();
			return;
		}
		else if (worker.currentPath.size() > 0) {
			// Casually move toward targetPos
			if (distanceToTargetPos >= 10) {
				worker.moveUnitTowardsInternalGoal();
			}
			// we hit our next sub-goal
			if (distanceToTargetPos < 5) {
				if (worker.connectionIdx < (int)worker.currentPath.size()) {
					worker.targetPos.x = (float)worker.currentPath[worker.connectionIdx].toNode.x;
					worker.targetPos.y = (float)worker.currentPath[worker.connectionIdx].toNode.y;
					worker.connectionIdx++;
				}
			}
			// we reached the end of our path, so therefore reset and make new path
			// and check if there are any nearby trees we can chop
			if (worker.connectionIdx >= (int)worker.currentPath.size() - 1) {
				worker.mapReference->fellTree(worker);

				// reset goal
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

	int randomNodeIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));

	this->currentTileIdx = getcurrentCorrespondingTile(mapReference->scoutedTiles->walkablePaths, this->pos);
	this->currentGoalTileIdx = getcurrentCorrespondingTile(mapReference->scoutedTiles->walkablePaths, this->goalPos);
	currentPath = ref->AStar(
					ref->walkablePaths[this->currentTileIdx],
					ref->walkablePaths[this->currentGoalTileIdx],
					ref->walkablePathsNeighboors);

	UnitBase::calculateNewPath();
}