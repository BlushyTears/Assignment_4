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
	distributingState = new DistributingState();

	targetIdeling = new TargetIdleState(idelingState);
	targetWoodcutting = new TargetCollectWoodState(collectingWoodState);
	targetIronCollecting = new TargetCollectIronState(collectingIronState);
	targetDistribution = new TargetDistributingState(distributingState);

	idleCheck = new IdleDecision();
	collectWoodCheck = new CollectWoodDecision();
	collectIronCheck = new CollectIronDecision();
	distributeCheck = new DistributingDecision();

	toIdle = new DecisionTreeTransition<Worker>();
	toWoodcutting = new DecisionTreeTransition<Worker>();
	toIronCollecting = new DecisionTreeTransition<Worker>();
	toDistributing = new DecisionTreeTransition<Worker>();

	idleCheck->trueNode = targetIdeling;
	idleCheck->falseNode = nullptr;
	collectWoodCheck->trueNode = targetWoodcutting;
	collectWoodCheck->falseNode = nullptr;
	collectIronCheck->trueNode = targetIronCollecting;
	collectIronCheck->falseNode = nullptr;
	distributeCheck->trueNode = targetDistribution;
	distributeCheck->falseNode = nullptr;

	toIdle->decisionTreeRoot = idleCheck;
	toWoodcutting->decisionTreeRoot = collectWoodCheck;
	toIronCollecting->decisionTreeRoot = collectIronCheck;
	toDistributing->decisionTreeRoot = distributeCheck;

	idelingState->transitions.push_back(toWoodcutting);
	idelingState->transitions.push_back(toIronCollecting);
	idelingState->transitions.push_back(toDistributing);
	collectingWoodState->transitions.push_back(toIdle);
	collectingWoodState->transitions.push_back(toIronCollecting);
	collectingWoodState->transitions.push_back(toDistributing);
	collectingIronState->transitions.push_back(toIdle);
	collectingIronState->transitions.push_back(toWoodcutting);
	collectingIronState->transitions.push_back(toDistributing);

	sm = new StateMachine<Worker>(collectingWoodState);

	unitSpeed = 0.2f;
}

Worker::~Worker() {
	delete sm;

	delete toIdle;
	delete toWoodcutting;
	delete toIronCollecting;

	delete idleCheck;
	delete collectWoodCheck;
	delete collectIronCheck;

	delete targetIdeling;
	delete targetWoodcutting;
	delete targetIronCollecting;

	delete idelingState;
	delete collectingWoodState;
	delete collectingIronState;
}

DecisionTreeNode<Worker>* DistributingDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount > 25 && worker.targetResourceTracker->workersDistributing < 3) {
		worker.targetResourceTracker->workersDistributing++;
		std::cout << "Unit decided to be a distributer" << std::endl;
		return this->trueNode;
	}
	return this->falseNode;
}
DecisionTreeNode<Worker>* IdleDecision::getBranch(Worker& worker) {
	if (false) {
		std::cout << "Unit decided to be idle" << std::endl;
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount < 50) {
		std::cout << "Unit decided to start gather Wood" << std::endl;
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectIronDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->ironOreCount < 30 && worker.targetResourceTracker->treeCount > 120) {
		std::cout << "Unit decided to start gather iron"  << std::endl;
		return this->trueNode;
	}
	return this->falseNode;
}

void DistributinAction::execute(Worker& worker) {
	if (worker.isCarryingWood) {
		if (worker.buildings.empty())
			return;

		Building* targetBuilding = nullptr;

		for (auto& building : worker.buildings) {
			if (building != nullptr) {
				if (!dynamic_cast<CoalMile*>(building) && building->treeCount < 10 && !building->isBuilt) {
					targetBuilding = building;
					break;
				}
			}
		}

		if (!targetBuilding)
			return;

		Vector2 buildingCenter = {
			targetBuilding->pos.x + (targetBuilding->tileSize / 2.0f),
			targetBuilding->pos.y + (targetBuilding->tileSize / 2.0f)
		};

		if (worker.goalPos.x != buildingCenter.x || worker.goalPos.y != buildingCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = buildingCenter;
		}

		if (Vector2Distance(worker.pos, buildingCenter) < 5.0f) {
			worker.isCarryingWood = false;
			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = Vector2{ -1, -1 };
			targetBuilding->treeCount++;
			return;
		}

		if (worker.currentPath.empty()) {
			worker.calculateNewPath();
			return;
		}
	}
	else {
		CoalMile* source = nullptr;
		for (auto& building : worker.buildings) {
			if (CoalMile* cm = dynamic_cast<CoalMile*>(building)) {
				if (cm->treeCount > 0) {
					source = cm;
					break;
				}
			}
		}

		if (!source)
			return;

		Vector2 sourceCenter = {
			source->pos.x + (source->tileSize / 2.0f),
			source->pos.y + (source->tileSize / 2.0f)
		};

		if (worker.goalPos.x != sourceCenter.x || worker.goalPos.y != sourceCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = sourceCenter;
		}

		if (Vector2Distance(worker.pos, sourceCenter) < 5.0f) {
			worker.isCarryingWood = true;
			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = Vector2{ -1, -1 };
			source->treeCount--;
			return;
		}

		if (worker.currentPath.empty()) {
			worker.calculateNewPath();
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

void IdleAction::execute(Worker& worker) {
	std::cout << "Unit is idle tree count: " << worker.targetResourceTracker->treeCount << std::endl;
}

void CollectWoodAction::execute(Worker& worker) {
	string trees = "Global Treecount: " + to_string(worker.targetResourceTracker->treeCount);
	DrawText(trees.c_str(), 300, 1130, 16, RED);

	string iron = "Global Iron ore count: " + to_string(worker.targetResourceTracker->ironOreCount);
	DrawText(iron.c_str(), 300, 1150, 16, RED);

	if (worker.isCarryingWood) {
		if (worker.buildings.empty())
			return;

		CoalMile* targetBuilding = nullptr;

		for (auto& building : worker.buildings) {
			if (CoalMile* cm = dynamic_cast<CoalMile*>(building)) {
				targetBuilding = cm;
				break;
			}
		}

		if (!targetBuilding)
			return;

		Vector2 buildingCenter = {
			targetBuilding->pos.x + (targetBuilding->tileSize / 2.0f),
			targetBuilding->pos.y + (targetBuilding->tileSize / 2.0f)
		};

		if (worker.goalPos.x != buildingCenter.x || worker.goalPos.y != buildingCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = buildingCenter;
		}

		if (Vector2Distance(worker.pos, buildingCenter) < 5.0f) {
			worker.isCarryingWood = false;
			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = Vector2{ -1, -1 };
			targetBuilding->treeCount++;
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
	if (worker.isCarryingIron) {
		if (worker.buildings.empty()) 
			return;
		// find our closest smelter if nothing else
		Smelter* targetSmelter = nullptr;
		for (auto& building : worker.buildings) {
			if (Smelter* s = dynamic_cast<Smelter*>(building)) {
				targetSmelter = s;
				break;
			}
		}
		if (!targetSmelter) 
			return;

		// get center of smelter
		Vector2 bCenter = { targetSmelter->pos.x + (targetSmelter->tileSize / 2.0f), targetSmelter->pos.y + (targetSmelter->tileSize / 2.0f) };

		if (worker.goalPos.x != bCenter.x || worker.goalPos.y != bCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = bCenter;
		}
		if (Vector2Distance(worker.pos, bCenter) < 8.0f) {
			worker.isCarryingIron = false;
			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = { -1, -1 };
			targetSmelter->ironOreCount++;
			return;
		}
		if (worker.currentPath.empty()) {
			worker.calculateNewPath();
			return;
		}
	}
	else {
		// if we are close to iron ore, pick up iron ore and move towards smelter
		if (worker.goalPos.x != -1 && Vector2Distance(worker.pos, worker.goalPos) < 2.0f) {
			bool foundIron = false;

			for (int i = 0; i < (int)worker.mapReference->ironOreIndices.size(); i++) {
				int tileIdx = worker.mapReference->ironOreIndices[i].first;
				if (worker.mapReference->renderedTiles[tileIdx].position.x == worker.goalPos.x &&
					worker.mapReference->renderedTiles[tileIdx].position.y == worker.goalPos.y) {

					auto& ironEntities = worker.mapReference->renderedTiles[tileIdx].occupyingEntities;
					for (int j = 0; j < (int)ironEntities.size(); j++) {
						if (ironEntities[j].entityType == eIronOre) {
							ironEntities.erase(ironEntities.begin() + j);
							foundIron = true;
							break;
						}
					}
					break;
				}
			}

			if (foundIron) {
				worker.isCarryingIron = true;
				worker.targetResourceTracker->ironOreCount++;
				worker.currentPath.clear();
				worker.connectionIdx = 0;
				worker.goalPos = { -1, -1 };
				return;
			}
			else {
				worker.goalPos = { -1, -1 };
				worker.currentPath.clear();
				worker.connectionIdx = 0;
			}
		}
		if (worker.goalPos.x == -1) {
			for (int i = 0; i < (int)worker.mapReference->ironOreIndices.size(); i++) {
				if (worker.mapReference->ironOreIndices[i].second == true) {
					worker.goalPos = worker.mapReference->renderedTiles[worker.mapReference->ironOreIndices[i].first].position;
					worker.mapReference->ironOreIndices[i].second = false;
					break;
				}
			}
		}
		if (worker.currentPath.empty() && worker.goalPos.x != -1) {
			worker.calculateNewPath();
			if (worker.currentPath.empty()) return;
		}
	}
	if (Vector2Distance(worker.pos, worker.targetPos) > 1.0f) worker.moveUnitTowardsInternalGoal();
	else {
		if (worker.connectionIdx < (int)worker.currentPath.size()) {
			worker.targetPos.x = (float)worker.currentPath[worker.connectionIdx].toNode.x;
			worker.targetPos.y = (float)worker.currentPath[worker.connectionIdx].toNode.y;
			worker.connectionIdx++;
		}
		else if (worker.goalPos.x != -1) worker.targetPos = worker.goalPos;
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