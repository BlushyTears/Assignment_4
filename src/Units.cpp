#include "Units.h"
#include "MapManager.h"
#include <cstdlib>

#include <iostream>

UnitBase::UnitBase(int _x, int _y, Map* _mp, ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _ur) {
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

			// then check if they are actually neighbooring tiles using this compounded expression
			if (nx >= 0 && nx < 100 && ny >= 0 && ny < 100) {
				Tile& tile = mapReference->renderedTiles[ny * 100 + nx];

				if (!tile.hasBeenScouted) {
					tile.hasBeenScouted = true;

					if (tile.tileType == Trees)
						mapReference->scoutedTreeIndices.push_back(ny * 100 + nx);

					if (tile.tileType == Grass || tile.tileType == Swamp) {
						mapReference->scoutedTiles->walkablePaths.push_back(tile.position);
						mapReference->scoutedTiles->graph->addLatestNodeToGraph(mapReference->scoutedTiles->walkablePaths);
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

// For now coal builders just behave like scouts
void CoalWorker::calculateNewPath() {
	auto ref = mapReference->scoutedTiles;

	int randomNodeIdx = getRandomNumber(0, (ref->walkablePaths.size() - 1));
	currentTileIdx = getcurrentCorrespondingTile(ref->walkablePaths, this->pos);

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
