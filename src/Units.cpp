#include "Units.h"
#include "MapManager.h"
#include <cstdlib>

#include <iostream>

UnitBase::UnitBase(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur, std::vector<Building*>& _bu) : buildings(_bu) {
	pos.x = _x;
	pos.y = _y;
	targetPos = pos;
	goalPos = pos;
	mapReference = _mp;
	renderedTiles = &mapReference->renderedTiles;
	targetResourceTracker = _rt;
	_unitsReference = _ur;
}

void UnitBase::AwaitNewPath() {
	if (isAwaitingNewPath)
		return;

	//std::cout << "Searching for new path";
	mapReference->searchQueue.emplace(this);
	isAwaitingNewPath = true;
}

void UnitBase::testTile() {
	// first convert coordinates to tiles
	int tileX = (int)(pos.x / 10);
	int tileY = (int)(pos.y / 10);

	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			// then create indices for neighbooring tiles
			int nx = tileX + dx;
			int ny = tileY + dy;

			// then check if they are actually neighbooring tiles here
			if (nx >= 0 && nx < 100 && ny >= 0 && ny < 100) {
				Tile& tile = mapReference->renderedTiles[ny * 100 + nx];

				if (!tile.hasBeenScouted) {
					tile.hasBeenScouted = true;

					if (tile.tileType == Trees)
						mapReference->scoutedTreeTileIndices.push_back(ny * 100 + nx);

					// ugly, implicit assumption that iron ores exist if it's not a tree tile
					if (tile.tileType == Grass || tile.tileType == Swamp) {
						if (mapReference->renderedTiles[ny * 100 + nx].occupyingEntities.size() > 0) {
							mapReference->ironOreIndices.push_back({ ny * 100 + nx, true });
						}
					}

					if (tile.tileType == Grass || tile.tileType == Swamp || tile.tileType == Trees) {
						mapReference->scoutedTiles->walkablePaths.push_back(tile.position);
						mapReference->scoutedTiles->graph->addLatestNodeToGraph(mapReference->scoutedTiles->walkablePaths);
						this->tileCountSinceComputeNeighboors++;
						mapReference->scoutedTiles->computeNewNeighboors(mapReference->scoutedTiles->walkablePaths.size() - 1);
					}
				}
			}
		}
	}
}

int UnitBase::getcurrentCorrespondingTile(std::vector<Vector2>& pathToCheck, Vector2& _unitPos) {
	if (pathToCheck.empty())
		return 0;

	float minDist = std::numeric_limits<float>::max();
	int closestIdx = 0;

	for (int i = 0; i < pathToCheck.size(); i++) {
		float dist = Vector2Distance(_unitPos, pathToCheck[i]);

		if (dist < minDist) {
			minDist = dist;
			closestIdx = i;
		}
	}

	return closestIdx;
}

void Scout::calculateNewPath() {
	auto ref = mapReference->accessableTiles;
	int randomNodeIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));

	currentTileIdx = getcurrentCorrespondingTile(mapReference->accessableTiles->walkablePaths, this->pos);

	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[randomNodeIdx],
		ref->walkablePathsNeighboors);

	UnitBase::calculateNewPath();
}

// Calculate path first, only then do we calculate next position
void Scout::commandUnit() {
	if (currentPath.size() == 0) {
		AwaitNewPath();
	}
	else {
		// Casually move toward targetPos
		if (Vector2Distance(pos, targetPos) > 10) {
			moveUnitTowardsInternalGoal();
			testTile();
		}
		// we hit our next goal
		if(Vector2Distance(pos, targetPos) < 5) {
			targetPos.x = (float)currentPath[connectionIdx].toNode.x;
			targetPos.y = (float)currentPath[connectionIdx].toNode.y;
			connectionIdx++;
		}
		// we reached the end of our path, so therefore reset and make new path
		if (connectionIdx >= currentPath.size() - 1) {
			currentPath.clear();
			connectionIdx = 0;
		}
	}
}

void Builder::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;

	if (targetBuilding != nullptr) {

		this->currentTileIdx = getcurrentCorrespondingTile(mapReference->scoutedTiles->walkablePaths, this->pos);
		this->currentGoalTileIdx = getcurrentCorrespondingTile(mapReference->scoutedTiles->walkablePaths, this->targetBuilding->pos);

		currentPath = ref->AStar(
			ref->walkablePaths[this->currentTileIdx],
			ref->walkablePaths[this->currentGoalTileIdx],
			ref->walkablePathsNeighboors);
	}

	UnitBase::calculateNewPath();
}

void Builder::commandUnit() {
	if (targetBuilding == nullptr) {
		for (auto& building : this->buildings) {
			if (!building->isBuilt) {
				this->targetBuilding = building;
				break;
			}
		}
	}
	if (targetBuilding != nullptr) {
		if (currentPath.size() == 0) {
			AwaitNewPath();
		}
		else {
			if (Vector2Distance(pos, targetPos) > 10) {
				moveUnitTowardsInternalGoal();
			}

			if (Vector2Distance(pos, targetPos) < 5) {
				if (connectionIdx < (int)currentPath.size()) {
					targetPos.x = (float)currentPath[connectionIdx].toNode.x;
					targetPos.y = (float)currentPath[connectionIdx].toNode.y;
					connectionIdx++;
				}
			}

			if (connectionIdx >= (int)currentPath.size()) {
				CoalMile* coalMile = dynamic_cast<CoalMile*>(targetBuilding);
				if (coalMile != nullptr && !coalMile->isBuilt) {
					if (coalMile->treeCount < coalMile->minTreesNeeded) {
						return;
					}
				}

				Smelter* smelter = dynamic_cast<Smelter*>(targetBuilding);
				if (smelter != nullptr && !smelter->isBuilt) {
					 if (smelter->treeCount < smelter->minTreesNeeded) {
					     return;
					 }
				}

				ArmSmith* armSmith = dynamic_cast<ArmSmith*>(targetBuilding);
				if (armSmith != nullptr && !armSmith->isBuilt) {
					 if (armSmith->treeCount < armSmith->minTreesNeeded) {
					     return;
					 }
				}

				if (!this->targetBuilding->isBuilt && !this->targetBuilding->isBuilding) {
					this->targetBuilding->isBuilding = true;
					this->targetBuilding->buildTimer.setNewTimer(5);
					this->targetBuilding->startBuildProcess();
				}

				if (this->targetBuilding->isBuilding) {
					this->targetBuilding->buildTimer.updateTimer();

					if (this->targetBuilding->buildTimer.hasTimerEnded()) {
						this->targetBuilding->isBuilt = true;
						this->targetBuilding->isBuilding = false;
						this->targetBuilding = nullptr;
						currentPath.clear();
						connectionIdx = 0;
					}
					else {
						return;
					}
				}
			}
		}
	}
}

void CoalWorker::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;

	int targetBuildingIdx = getcurrentCorrespondingTile(ref->walkablePaths, targetBuilding->pos);
	currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths, this->pos);

	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[targetBuildingIdx],
		ref->walkablePathsNeighboors);

	UnitBase::calculateNewPath();
}

void CoalWorker::commandUnit() {
	if (targetBuilding == nullptr) {
		for (auto& building : buildings) {
			CoalMile* cm = dynamic_cast<CoalMile*>(building);

			if (cm)
				targetBuilding = cm;
			break;
		}
		return;
	}
	if (currentPath.size() == 0) {
		AwaitNewPath();
	}
	else {
		// Casually move toward targetPos
		if (Vector2Distance(pos, targetPos) > 10) {
			moveUnitTowardsInternalGoal();
		}
		// we hit our next goal
		if (Vector2Distance(pos, targetPos) < 5) {
			targetPos.x = (float)currentPath[connectionIdx].toNode.x;
			targetPos.y = (float)currentPath[connectionIdx].toNode.y;
			connectionIdx++;
		}
		// we reached the end of our path, so therefore reset and make new path
		if (connectionIdx >= currentPath.size() - 1) {
			currentPath.clear();
			connectionIdx = 0;
		}
	}
}

void SmelterWorker::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;

	int targetBuildingIdx = getcurrentCorrespondingTile(ref->walkablePaths, targetBuilding->pos);
	currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths, this->pos);

	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[targetBuildingIdx],
		ref->walkablePathsNeighboors);

	UnitBase::calculateNewPath();
}

void SmelterWorker::commandUnit() {
	if (targetBuilding == nullptr) {
		for (auto& building : buildings) {
			Smelter* cm = dynamic_cast<Smelter*>(building);

			if (cm)
				targetBuilding = cm;
			break;
		}
		return;
	}
	if (currentPath.size() == 0) {
		AwaitNewPath();
	}
	else {
		// Casually move toward targetPos
		if (Vector2Distance(pos, targetPos) > 10) {
			moveUnitTowardsInternalGoal();
		}
		// we hit our next goal
		if (Vector2Distance(pos, targetPos) < 5) {
			targetPos.x = (float)currentPath[connectionIdx].toNode.x;
			targetPos.y = (float)currentPath[connectionIdx].toNode.y;
			connectionIdx++;
		}
		// we reached the end of our path, so therefore reset and make new path
		if (connectionIdx >= currentPath.size() - 1) {
			currentPath.clear();
			connectionIdx = 0;
		}
	}
}

void ArmSmithWorker::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;

	int targetBuildingIdx = getcurrentCorrespondingTile(ref->walkablePaths, targetBuilding->pos);
	currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths, this->pos);

	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[targetBuildingIdx],
		ref->walkablePathsNeighboors);

	UnitBase::calculateNewPath();
}

void ArmSmithWorker::commandUnit() {
	if (targetBuilding == nullptr) {
		for (auto& building : buildings) {
			ArmSmith* s = dynamic_cast<ArmSmith*>(building);

			if (s)
				targetBuilding = s;
			break;
		}
		return;
	}
	if (currentPath.size() == 0) {
		AwaitNewPath();
	}
	else {
		// Casually move toward targetPos
		if (Vector2Distance(pos, targetPos) > 10) {
			moveUnitTowardsInternalGoal();
		}
		// we hit our next goal
		if (Vector2Distance(pos, targetPos) < 5) {
			targetPos.x = (float)currentPath[connectionIdx].toNode.x;
			targetPos.y = (float)currentPath[connectionIdx].toNode.y;
			connectionIdx++;
		}
		// we reached the end of our path, so therefore reset and make new path
		if (connectionIdx >= currentPath.size() - 1) {
			currentPath.clear();
			connectionIdx = 0;
		}
	}
}
