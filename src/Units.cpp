#include "Units.h"
#include "MapManager.h"
#include <cstdlib>

#include <iostream>

UnitBase::UnitBase(int _x, int _y, Map* _mp) {
	pos.x = _x;
	pos.y = _y;
	targetPos = pos;
	mapReference = _mp;
	renderedTiles = &mapReference->renderedTiles;
}

void UnitBase::testTile() {
	for (auto& tile : *renderedTiles) {
		if (tile.isUnitWithinTile(*this, 25) && !tile.hasBeenScouted) {
			tile.hasBeenScouted = true;
			mapReference->accessableTiles->scoutedPaths.push_back(tile.position);
		}
	}
}

int UnitBase::getCorrespondingTile(std::vector<Vector2>& pathToCheck) {
	auto walkablePathsRef = mapReference->accessableTiles->walkablePaths;

	for (int i = 0; i < pathToCheck.size(); i++) {
		if (pos.x >= pathToCheck[i].x && pos.x < pathToCheck[i].x + TILE_SIZE &&
			pos.y >= pathToCheck[i].y && pos.y < pathToCheck[i].y + TILE_SIZE) {
			return i;
		}
	}
	return 0;
}

// Calculate path first, only then do we calculate next position
void Scout::moveUnit() {
	// We start with an empty path
	//std::cout << "path size " << currentPath.size() << std::endl;

	if (currentPath.size() == 0) {
		auto ref = mapReference->accessableTiles;
		int randomNodeIdx = getRandomNumber(0, ref->walkablePaths.size() - 1);
		int tileIdx = getCorrespondingTile(mapReference->accessableTiles->walkablePaths);
		currentPath = ref->dijkstra(
			ref->walkablePaths[tileIdx],
			ref->walkablePaths[randomNodeIdx],
			ref->walkablePaths,
			ref->walkablePathsNeighboors);
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

void Worker::moveUnit() {
	// We start with an empty path
	//std::cout << "path size: " << currentPath.size() << std::endl;

	auto ref = mapReference->accessableTiles;
	if (currentPath.size() == 0 && ref->scoutedPaths.size() >= 1) {
		int randomNodeIdx = getRandomNumber(0, ref->scoutedPaths.size() - 1);
		int tileIdx = getCorrespondingTile(mapReference->accessableTiles->scoutedPaths);
		currentPath = ref->AStar(
			ref->scoutedPaths[tileIdx],
			ref->scoutedPaths[randomNodeIdx],
			ref->scoutedPaths,
			ref->ScoutedPathsNeighboors);
	}
	else {
		/// Casually move toward targetPos
		if (Vector2Distance(pos, targetPos) > 10) {
			moveUnitTowardsInternalGoal();
		}
		// we hit our next goal
		if (Vector2Distance(pos, targetPos) < 5) {
			connectionIdx++;
			targetPos.x = (float)currentPath[connectionIdx].toNode.x;
			targetPos.y = (float)currentPath[connectionIdx].toNode.y;
		}
		// we reached the end of our path, so therefore reset and make new path
		if (connectionIdx >= currentPath.size() - 1) {
			currentPath.clear();
			connectionIdx = 0;
			std::cout << "Path ended" << std::endl;
		}
	}
}

