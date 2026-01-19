#include "Units.h"
#include "MapManager.h"
#include <cstdlib>

#include <iostream>

UnitBase::UnitBase(int _x, int _y, Map* _mp, ResourceTracker* _rt) {
	pos.x = _x;
	pos.y = _y;
	targetPos = pos;
	mapReference = _mp;
	renderedTiles = &mapReference->renderedTiles;
	targetResourceTracker = _rt;
}

void UnitBase::AwaitNewPath() {
	if (isAwaitingNewPath)
		return;

	//std::cout << "Searching for new path";
	mapReference->searchQueue.emplace(this);
	isAwaitingNewPath = true;
}

void UnitBase::testTile() {
	int idx = 0;
	for (auto& tile : *renderedTiles) {
		if (tile.isUnitWithinTile(*this, 25) && !tile.hasBeenScouted) {
			tile.hasBeenScouted = true;

			if (tile.tileType == Trees) {
				mapReference->scoutedTreeIndices.push_back(idx);
			}

			if (tile.tileType == Grass || tile.tileType == Swamp) {
				mapReference->scoutedTiles->walkablePaths.push_back(tile.position);
				mapReference->scoutedTiles->graph->addLatestNodeToGraph(mapReference->scoutedTiles->walkablePaths);
				mapReference->scoutedTiles->computeNewNeighboors(mapReference->scoutedTiles->walkablePaths.size() - 1);
			}
		}
		idx++;
	}
}

int UnitBase::getcurrentCorrespondingTile(std::vector<Vector2>& pathToCheck) {
	if (pathToCheck.empty())
		return 0;

	float minDist = std::numeric_limits<float>::max();
	int closestIdx = 0;

	for (int i = 0; i < pathToCheck.size(); i++) {
		float dist = Vector2Distance(pos, pathToCheck[i]);

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

	currentTileIdx = getcurrentCorrespondingTile(mapReference->accessableTiles->walkablePaths);

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

// For now coal builders just behave like scouts
void CoalWorker::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;

	int randomNodeIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));
	currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths);

	currentPath = ref->AStar(
		ref->walkablePaths[currentTileIdx],
		ref->walkablePaths[randomNodeIdx],
		ref->walkablePathsNeighboors);

	UnitBase::calculateNewPath();
}

void CoalWorker::commandUnit() {
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
