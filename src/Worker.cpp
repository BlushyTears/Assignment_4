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
	goalIdx = -1;
}

DecisionTreeNode<Worker>* IdleDecision::getBranch(Worker& worker) {
	// Only start chopping trees once the map is a certain size (testing reasons)
	if (/* worker.targetResourceTracker->treeCount >= 500 */ worker.goalIdx == -1) {
		int nextPotentialGoal = worker.mapReference->getNearestTreeIdx(worker);

		if (nextPotentialGoal != -1) {
			worker.goalIdx = nextPotentialGoal;
			return this->falseNode;
		}

		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.goalIdx == -1) {
		worker.goalIdx = worker.mapReference->getNearestTreeIdx(worker);
	}

	// found a tree
	if (worker.goalIdx != -1) {
		return this->trueNode;
	}
	return this->falseNode;
}

void IdleAction::execute(Worker& worker) {
	std::cout << "Unit is idle tree count: " << worker.targetResourceTracker->treeCount << std::endl;
}

void CollectWoodAction::execute(Worker& worker) {
	//std::cout << "Unit is cutting wood. goal index: " << worker.goalIdx << std::endl;
	if (worker.goalIdx == -1) {
		worker.goalIdx = worker.mapReference->getNearestTreeIdx(worker);
		return;
	}
	else {
		if (worker.currentPath.size() == 0) {
			worker.AwaitNewPath();
			return;
		}
		else if (worker.currentPath.size() > 0) {
			// Casually move toward targetPos
			if (Vector2Distance(worker.pos, worker.targetPos) > 10) {
				worker.moveUnitTowardsInternalGoal();
			}
			// we hit our next goal
			if (Vector2Distance(worker.pos, worker.targetPos) < 5) {
				if (worker.connectionIdx < (int)worker.currentPath.size()) {
					worker.targetPos.x = (float)worker.currentPath[worker.connectionIdx].toNode.x;
					worker.targetPos.y = (float)worker.currentPath[worker.connectionIdx].toNode.y;
					worker.connectionIdx++;
				}
			}
			// we reached the end of our path, so therefore reset and make new path
			if (worker.connectionIdx >= (int)worker.currentPath.size()) {
				Vector2 arrivalPos = worker.pos;
				int actualTreeTileIdx = -1;

				float closestTree = 200.0f;
				for (int treeMapIdx : worker.mapReference->scoutedTreeIndices) {
					float d = Vector2Distance(arrivalPos, worker.mapReference->renderedTiles[treeMapIdx].position);

					if (d < closestTree) {
						closestTree = d;
						actualTreeTileIdx = treeMapIdx;
					}
				}

				if (actualTreeTileIdx != -1) {

					worker.mapReference->fellTree(actualTreeTileIdx);
					worker.targetResourceTracker->treeCount++;

					// reset goal
					worker.goalIdx = -1;
					worker.currentPath.clear();
					worker.connectionIdx = 0;

					worker.currentPath.clear();
					worker.connectionIdx = 0;
				}
				else {
					worker.goalIdx = -1; 
					worker.currentPath.clear();
					worker.connectionIdx = 0;
				}
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

	if (ref->walkablePaths.empty())
		return;

	if (shouldWander) {
		targetIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));
	}
	else {
		targetIdx = goalIdx;
	}

	if (targetIdx < 0 || targetIdx >= ref->walkablePaths.size()) 
		return;


	currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths);
	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[targetIdx], // position of targetIndex in full map. Vector2
		ref->walkablePathsNeighboors);

	if(currentPath.size() > 0) {}
	//std::cout << "path" << std::endl;
	UnitBase::calculateNewPath();
}
