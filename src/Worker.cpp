#include "Units.h"
#include "Worker.h"
#include "GameManager.h"

#include <cstdlib>
#include <iostream>

// set up fsm here
Worker::Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu) 
	: UnitBase(_x, _y, _mp, _rt, _ur, _bu) {
	idelingState = new IdleState();
	collectingWoodState = new CollectWoodState();
	collectingIronState = new CollectIronState();

	targetIdeling = new TargetIdleState(idelingState);
	targetWoodcutting = new TargetCollectWoodState(collectingWoodState);
	targetIronCollecting = new TargetCollectIronState(collectingIronState);
	
	idleCheck = new IdleDecision();
	collectWoodCheck = new CollectWoodDecision();
	collectIronCheck = new CollectIronDecision();

	toIdle = new DecisionTreeTransition<Worker>();
	toWoodcutting = new DecisionTreeTransition<Worker>();
	toIronCollecting = new DecisionTreeTransition<Worker>();

	idleCheck->trueNode = targetIdeling;
	idleCheck->falseNode = nullptr;
	collectWoodCheck->trueNode = targetWoodcutting;
	collectWoodCheck->falseNode = nullptr;
	collectIronCheck->trueNode = targetIronCollecting;
	collectIronCheck->falseNode = nullptr;

	toIdle->decisionTreeRoot = idleCheck;
	toWoodcutting->decisionTreeRoot = collectWoodCheck;
	toIronCollecting->decisionTreeRoot = collectIronCheck;

	idelingState->transitions.push_back(toWoodcutting);
	idelingState->transitions.push_back(toIronCollecting);
	collectingWoodState->transitions.push_back(toIdle);
	collectingWoodState->transitions.push_back(toIronCollecting);
	collectingIronState->transitions.push_back(toIdle);
	collectingIronState->transitions.push_back(toWoodcutting);

	sm = new StateMachine<Worker>(collectingWoodState);

	unitSpeed = 0.2f;
}


DecisionTreeNode<Worker>* IdleDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount > 500 && worker.targetResourceTracker->ironOreCount > 50 && !worker.isCarryingWood) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount < 45) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectIronDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->ironOreCount < 30 && worker.targetResourceTracker->treeCount > 60) {
		return this->trueNode;
	}
	return this->falseNode;
}

void IdleAction::execute(Worker& worker) {
	std::cout << "Unit is idle tree count: " << worker.targetResourceTracker->treeCount << std::endl;
}

void CollectWoodAction::execute(Worker& worker) {
	if (worker.isCarryingWood) {
		if (worker.buildings.empty())
			return;

		Building* targetBuilding = nullptr;
		CoalMile* primaryCoalMile = nullptr;
		Smelter* secondarySmelter = nullptr;

		for (auto& building : worker.buildings) {
			if (CoalMile* cm = dynamic_cast<CoalMile*>(building)) {
				primaryCoalMile = cm;
			}
			if (Smelter* s = dynamic_cast<Smelter*>(building)) {
				secondarySmelter = s;
			}
		}

		if (primaryCoalMile && primaryCoalMile->treeCount < 15) {
			targetBuilding = primaryCoalMile;
		}
		else if (secondarySmelter && secondarySmelter->treeCount < 15) {
			targetBuilding = secondarySmelter;
		}
		else {
			targetBuilding = primaryCoalMile;
		}

		if (!targetBuilding) return;

		Vector2 buildingCenter = {
			targetBuilding->pos.x + (targetBuilding->tileSize / 2.0f),
			targetBuilding->pos.y + (targetBuilding->tileSize / 2.0f)
		};

		if (worker.goalPos.x != buildingCenter.x || worker.goalPos.y != buildingCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = buildingCenter;
		}

		if (Vector2Distance(worker.pos, buildingCenter) < 2.0f) {
			worker.isCarryingWood = false;
			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = Vector2{ -1, -1 };

			if (CoalMile* cm = dynamic_cast<CoalMile*>(targetBuilding)) {
				cm->treeCount++;
			}
			else if (Smelter* s = dynamic_cast<Smelter*>(targetBuilding)) {
				s->treeCount++;
			}
			return;
		}

		if (worker.currentPath.empty()) {
			worker.calculateNewPath();
			return;
		}
	}
	else {
		if (worker.mapReference->tryToFellTree(worker)) {
			worker.targetResourceTracker->treeCount++;
			worker.isCarryingWood = true;
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
			if (worker.currentPath.empty())
				return;
		}
	}

	if (Vector2Distance(worker.pos, worker.targetPos) > 1.0f) {
		worker.moveUnitTowardsInternalGoal();
	}
	else {
		if (worker.connectionIdx < (int)worker.currentPath.size()) {
			worker.targetPos.x = (float)worker.currentPath[worker.connectionIdx].toNode.x;
			worker.targetPos.y = (float)worker.currentPath[worker.connectionIdx].toNode.y;
			worker.connectionIdx++;
		}
		else if (worker.goalPos.x != -1) {
			worker.targetPos = worker.goalPos;
		}
	}
}

void CollectIronAction::execute(Worker& worker) {
	worker.goalPos = Vector2{ 500, 500 };

	DrawCircle(500, 500, 25, RED);
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