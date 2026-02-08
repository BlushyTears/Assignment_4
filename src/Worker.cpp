#include "Units.h"
#include "Worker.h"
#include "GameManager.h"

#include <cstdlib>
#include <iostream>

Worker::Worker(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu)
	: UnitBase(_x, _y, _mp, _rt, _ur, _bu)
{
	collectingWoodState = new CollectWoodState();
	collectingIronState = new CollectIronState();
	distributingState = new DistributingState();

	targetWoodcutting = new TargetCollectWoodState(collectingWoodState);
	targetIronCollecting = new TargetCollectIronState(collectingIronState);
	targetDistribution = new TargetDistributingState(distributingState);

	collectWoodCheck = new CollectWoodDecision();
	collectIronCheck = new CollectIronDecision();
	distributeCheck = new DistributingDecision();

	toWoodcutting = new DecisionTreeTransition<Worker>();
	toIronCollecting = new DecisionTreeTransition<Worker>();
	toDistributing = new DecisionTreeTransition<Worker>();

	collectWoodCheck->trueNode = targetWoodcutting;
	collectWoodCheck->falseNode = nullptr;

	collectIronCheck->trueNode = targetIronCollecting;
	collectIronCheck->falseNode = nullptr;

	distributeCheck->trueNode = targetDistribution;
	distributeCheck->falseNode = nullptr;

	toWoodcutting->decisionTreeRoot = collectWoodCheck;
	toIronCollecting->decisionTreeRoot = collectIronCheck;
	toDistributing->decisionTreeRoot = distributeCheck;

	collectingWoodState->transitions.push_back(toIronCollecting);
	collectingWoodState->transitions.push_back(toDistributing);

	collectingIronState->transitions.push_back(toWoodcutting);
	collectingIronState->transitions.push_back(toDistributing);

	sm = new StateMachine<Worker>(collectingWoodState);

	unitSpeed = 0.2f;
}

Worker::~Worker()
{
	delete sm;
	delete toWoodcutting;
	delete toIronCollecting;
	delete toDistributing;
	delete collectWoodCheck;
	delete collectIronCheck;
	delete distributeCheck;
	delete targetWoodcutting;
	delete targetIronCollecting;
	delete targetDistribution;
	delete collectingWoodState;
	delete collectingIronState;
	delete distributingState;
}

DecisionTreeNode<Worker>* DistributingDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount > 25 && worker.targetResourceTracker->workersDistributing < 5) {
		worker.targetResourceTracker->workersDistributing++;
		std::cout << "Unit decided to be a distributer" << std::endl;
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectWoodDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->treeCount <= 40 && !worker.isCarryingIron) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* CollectIronDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->ironOreCount <= 60 && worker.targetResourceTracker->treeCount > 80 && !worker.isCarryingWood) {
		return this->trueNode;
	}
	return this->falseNode;
}

DecisionTreeNode<Worker>* TrainUnitDecision::getBranch(Worker& worker) {
	if (worker.targetResourceTracker->ironSwordCount > 0) {
		return this->falseNode;
	}
	return this->falseNode;
}

void DistributinAction::execute(Worker& worker)
{
	string iron = "iron ore count: " + to_string(worker.targetResourceTracker->ironOreCount);
	DrawText(iron.c_str(), 300, 1110, 16, BLUE);

	string trees = "Global Treecount: " + to_string(worker.targetResourceTracker->treeCount);
	DrawText(trees.c_str(), 300, 1130, 16, BLUE);

	string coal = "Coal mile tree count: " + to_string(worker.targetResourceTracker->woodInCoalMile);
	DrawText(coal.c_str(), 300, 1150, 16, BLUE);

	if (worker.isCarryingWood || worker.isCarryingCoal ||
		worker.isCarryingIronBar || worker.isCarryingIronArrow) {
		if (worker.buildings.empty())
			return;

		Building* targetBuilding = nullptr;

		// ugly to cast all the time instead of cache the buildings
		// but in a reality when buildings are changing, this is somehow justified (Although maybe it should be done less often)
		if (worker.isCarryingWood) {
			for (auto& building : worker.buildings) {
				if (building
					&& !dynamic_cast<CoalMile*>(building)
					&& building->treeCount < building->minTreesNeeded
					&& !building->isBuilding) {
					targetBuilding = building;
					break;
				}
			}
		}
		else if (worker.isCarryingIronArrow) {
			for (auto& building : worker.buildings) {
				if (ArmSmith* a = dynamic_cast<ArmSmith*>(building)) {
					if (!a->isBuilt) {
						targetBuilding = a;
						break;
					}
				}
			}
		}
		else if (worker.isCarryingIronBar) {
			for (auto& building : worker.buildings) {
				if (ArmSmith* a = dynamic_cast<ArmSmith*>(building)) {
					targetBuilding = a;
					break;
				}
			}
		}
		else if (worker.isCarryingCoal) {
			for (auto& building : worker.buildings) {
				if (Smelter* s = dynamic_cast<Smelter*>(building)) {
					if (s->coalCount < 10) {
						targetBuilding = s;
						break;
					}
				}
				if (ArmSmith* a = dynamic_cast<ArmSmith*>(building)) {
					if (a->coalCount < 15) {
						targetBuilding = a;
						break;
					}
				}
			}
		}

		if (!targetBuilding) {
			worker.isCarryingWood = false;
			worker.isCarryingCoal = false;
			worker.isCarryingIronBar = false;
			worker.isCarryingIronArrow = false;

			worker.goalPos = { -1, -1 };
			return;
		}

		// here we check if we are close to our target building (Could be any at this point)
		Vector2 buildingCenter = {
			targetBuilding->pos.x + targetBuilding->tileSize * 0.5f,
			targetBuilding->pos.y + targetBuilding->tileSize * 0.5f
		};

		if (worker.goalPos.x != buildingCenter.x || worker.goalPos.y != buildingCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = buildingCenter;
		}

		// check what we're carrying and compare against the building we're closest to
		if (Vector2Distance(worker.pos, buildingCenter) < 8.0f) {
			if (worker.isCarryingWood)
				targetBuilding->treeCount++;
			else if (worker.isCarryingCoal) {
				if (Smelter* s = dynamic_cast<Smelter*>(targetBuilding))
					s->coalCount++;
				else if (ArmSmith* a = dynamic_cast<ArmSmith*>(targetBuilding))
					a->coalCount++;
			}
			else if (worker.isCarryingIronArrow) {
				if (ArmSmith* a = dynamic_cast<ArmSmith*>(targetBuilding)) 
					a->ironArrowCount++;
			}
			else if (worker.isCarryingIronBar) {
				if (ArmSmith* a = dynamic_cast<ArmSmith*>(targetBuilding))
					a->ironBarCount++;
			}

			worker.isCarryingWood = false;
			worker.isCarryingCoal = false;
			worker.isCarryingIronBar = false;
			worker.isCarryingIronArrow = false;

			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = { -1, -1 };
			return;
		}

		if (worker.currentPath.empty()) {
			worker.calculateNewPath();
			return;
		}
	}

	else {
		Building* source = nullptr;
		int pickupType = -1;

		bool woodNeeded = false;
		for (auto& b : worker.buildings) {
			if (b &&
				!dynamic_cast<CoalMile*>(b) &&
				b->treeCount < b->minTreesNeeded &&
				!b->isBuilding) {
				woodNeeded = true;
				break;
			}
		}

		if (woodNeeded) {
			for (auto& b : worker.buildings) {
				if (CoalMile* cm = dynamic_cast<CoalMile*>(b)) {
					if (cm->treeCount > 0) {
						source = cm;
						pickupType = 0;
						break;
					}
				}
			}
		}
		else {
			for (auto& b : worker.buildings) {
				if (Smelter* s = dynamic_cast<Smelter*>(b)) {
					if (s->ironBarCount > 0) {
						source = s;
						pickupType = 2;
						break;
					}
					if (s->ironArrowCount > 0) {
						source = s;
						pickupType = 3;
						break;
					}
				}
				if (CoalMile* cm = dynamic_cast<CoalMile*>(b)) {
					if (cm->coalCount > 0) {
						bool coalNeeded = false;
						for (auto& dest : worker.buildings) {
							if (Smelter* ds = dynamic_cast<Smelter*>(dest)) {
								if (ds->coalCount < 10) { coalNeeded = true; break; }
							}
							if (ArmSmith* da = dynamic_cast<ArmSmith*>(dest)) {
								if (da->coalCount < 15) { coalNeeded = true; break; }
							}
						}

						if (coalNeeded) {
							source = cm;
							pickupType = 1;
							break;
						}
					}
				}
			}
		}

		if (!source) {
			worker.goalPos = { -1, -1 };
			return;
		}

		Vector2 sourceCenter = {
			source->pos.x + source->tileSize * 0.5f,
			source->pos.y + source->tileSize * 0.5f
		};

		if (worker.goalPos.x != sourceCenter.x || worker.goalPos.y != sourceCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = sourceCenter;
		}

		if (Vector2Distance(worker.pos, sourceCenter) < 8.0f) {
			if (pickupType == 0) {
				worker.isCarryingWood = true;
				((CoalMile*)source)->treeCount--;
			}
			else if (pickupType == 1) {
				worker.isCarryingCoal = true;
				((CoalMile*)source)->coalCount--;
			}
			else if (pickupType == 2) {
				worker.isCarryingIronBar = true;
				((Smelter*)source)->ironBarCount--;
			}
			else if (pickupType == 3) {
				worker.isCarryingIronArrow = true;
				((Smelter*)source)->ironArrowCount--;
			}

			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = { -1, -1 };
			return;
		}

		if (worker.currentPath.empty()) {
			worker.calculateNewPath();
			return;
		}
	}

	if (Vector2Distance(worker.pos, worker.targetPos) > 1.0f)
		worker.moveUnitTowardsInternalGoal();
	else if (worker.connectionIdx < (int)worker.currentPath.size()) {
		worker.targetPos = {
			(float)worker.currentPath[worker.connectionIdx].toNode.x,
			(float)worker.currentPath[worker.connectionIdx].toNode.y
		};
		worker.connectionIdx++;
	}
	else if (worker.goalPos.x != -1)
		worker.targetPos = worker.goalPos;
}


void CollectWoodAction::execute(Worker& worker)
{
	string iron = "iron ore count: " + to_string(worker.targetResourceTracker->ironOreCount);
	DrawText(iron.c_str(), 300, 1110, 16, RED);

	string trees = "Global Treecount: " + to_string(worker.targetResourceTracker->treeCount);
	DrawText(trees.c_str(), 300, 1130, 16, RED);

	string coal = "Coal mile tree count: " + to_string(worker.targetResourceTracker->woodInCoalMile);
	DrawText(coal.c_str(), 300, 1150, 16, RED);

	if (worker.isCarryingWood)
	{
		CoalMile* targetBuilding = nullptr;
		for (auto& building : worker.buildings) {
			if (CoalMile* cm = dynamic_cast<CoalMile*>(building)) { 
				targetBuilding = cm;
				break;
			}
		}
		if (!targetBuilding) { 
			worker.isCarryingWood = false; 
			return;
		}

		Vector2 buildingCenter = { targetBuilding->pos.x + (targetBuilding->tileSize / 2.0f), targetBuilding->pos.y + (targetBuilding->tileSize / 2.0f) };

		if (worker.goalPos.x != buildingCenter.x || worker.goalPos.y != buildingCenter.y) {
			worker.currentPath.clear();
			worker.goalPos = buildingCenter;
		}
		if (Vector2Distance(worker.pos, buildingCenter) < 8.0f) {
			worker.isCarryingWood = false;
			worker.currentPath.clear();
			worker.connectionIdx = 0;
			worker.goalPos = { -1, -1 };
			targetBuilding->treeCount++;
			return;
		}
		if (worker.currentPath.empty()) { worker.calculateNewPath(); 
			return;
		}
	}
	else {
		if (worker.mapReference->tryToFellTree(worker)) {
			worker.targetResourceTracker->treeCount++;
			worker.isCarryingWood = true;
			int sameTileIdx = worker.treeTileTargetIdx;
			worker.mapReference->removeTreeByIndex(worker.treeTileTargetIdx, worker.treeTargetIdx);
			worker.treeTargetIdx = worker.treeTileTargetIdx = -1;
			worker.currentPath.clear();
			worker.connectionIdx = 0;
			if (sameTileIdx != -1) {
				for (auto& entity : worker.mapReference->renderedTiles[sameTileIdx].occupyingEntities) {
					if (entity.entityType == eTree && !entity.reserved)
					{
						entity.reserved = true;
						worker.treeTargetIdx = entity.idx;
						worker.treeTileTargetIdx = sameTileIdx;
						worker.goalPos = worker.mapReference->renderedTiles[sameTileIdx].position + entity.tileOffset;
						return;
					}
				}
			}
			worker.goalPos = { -1, -1 };
			return;
		}
		worker.chopTimer.updateTimer();
		if (worker.currentPath.empty()) {
			worker.goalPos = worker.mapReference->getNearestTreePos(worker);
			if (worker.goalPos.x != -1) 
				worker.calculateNewPath();

			if (worker.currentPath.empty()) 
				return;
		}
	}

	if (Vector2Distance(worker.pos, worker.targetPos) > 1.0f) 
		worker.moveUnitTowardsInternalGoal();

	else if (worker.connectionIdx < (int)worker.currentPath.size()) {
		worker.targetPos = { (float)worker.currentPath[worker.connectionIdx].toNode.x, (float)worker.currentPath[worker.connectionIdx].toNode.y };
		worker.connectionIdx++;
	}
	else if (worker.goalPos.x != -1)
		worker.targetPos = worker.goalPos;
}

void CollectIronAction::execute(Worker& worker) {

	string iron = "iron ore count: " + to_string(worker.targetResourceTracker->ironOreCount);
	DrawText(iron.c_str(), 300, 1110, 16, ORANGE);

	string trees = "Global Treecount: " + to_string(worker.targetResourceTracker->treeCount);
	DrawText(trees.c_str(), 300, 1130, 16, ORANGE);

	string coal = "Coal mile tree count: " + to_string(worker.targetResourceTracker->woodInCoalMile);
	DrawText(coal.c_str(), 300, 1150, 16, ORANGE);

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
			return;
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
	if (Vector2Distance(worker.pos, worker.targetPos) > 1.0f)
		worker.moveUnitTowardsInternalGoal();
	else {
		if (worker.connectionIdx < (int)worker.currentPath.size()) {
			worker.targetPos.x = (float)worker.currentPath[worker.connectionIdx].toNode.x;
			worker.targetPos.y = (float)worker.currentPath[worker.connectionIdx].toNode.y;
			worker.connectionIdx++;
		}
		else if (worker.goalPos.x != -1) 
			worker.targetPos = worker.goalPos;
	}
}

void TrainUnitAction::execute(Worker& worker)
{
	// unit is at training camp and therefore let game manager train it
	if (worker.trainingCamp != nullptr && worker.trainingCamp->isWorkerAvailable) {
		return;
	}

	if (worker.trainingCamp == nullptr) {
		for (auto& building : worker.buildings) {
			if (TrainingCamp* t = dynamic_cast<TrainingCamp*>(building)) {
				building = t;
				break;
			}
		}
	}

	if (worker.trainingCamp != nullptr) {
		Vector2 bCenter = { worker.trainingCamp->pos.x + (worker.trainingCamp->tileSize / 2.0f), worker.trainingCamp->pos.y + (worker.trainingCamp->tileSize / 2.0f) };

		if (worker.goalPos.x != bCenter.x || worker.goalPos.y != bCenter.y) {
			worker.goalPos = bCenter;
			worker.trainingCamp->isWorkerAvailable = true;
			worker.isReadyToTrain = true;
			return;
		}
	}

	if (Vector2Distance(worker.pos, worker.targetPos) > 4.0f) 
		worker.moveUnitTowardsInternalGoal();
	else if (worker.connectionIdx < (int)worker.currentPath.size()) {
		worker.targetPos = { (float)worker.currentPath[worker.connectionIdx].toNode.x, (float)worker.currentPath[worker.connectionIdx].toNode.y };
		worker.connectionIdx++;
	}
	else if (worker.goalPos.x != -1) 
			worker.targetPos = worker.goalPos;
}

void Worker::commandUnit()
{
	plans = sm->update(*this);
	for (auto& action : plans) 
		action->execute(*this);
}

void Worker::calculateNewPath()
{
	if (this->goalPos.x == -1) 
		return;

	auto ref = mapReference->scoutedTiles;
	this->currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths, this->pos);
	this->currentGoalTileIdx = getcurrentCorrespondingTile(ref->walkablePaths, this->goalPos);
	currentPath = ref->AStar(ref->walkablePaths[this->currentTileIdx], ref->walkablePaths[this->currentGoalTileIdx], ref->walkablePathsNeighboors);

	if (!currentPath.empty()) {
		this->connectionIdx = 0;
		this->targetPos.x = (float)this->currentPath[this->connectionIdx].toNode.x;
		this->targetPos.y = (float)this->currentPath[this->connectionIdx].toNode.y;
	}
	UnitBase::calculateNewPath();
}