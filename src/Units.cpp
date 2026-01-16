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

void UnitBase::AwaitNewPath()
{
	if (isAwaitingNewPath)
		return;

	mapReference->searchQueue.emplace(this);
	isAwaitingNewPath = true;
}

void UnitBase::testTile() {
	for (auto& tile : *renderedTiles) {
		if (tile.isUnitWithinTile(*this, 25) && !tile.hasBeenScouted) {
			tile.hasBeenScouted = true;
			//mapReference->accessableTiles->scoutedPaths.push_back(tile.position);
		}
	}
}

// slow but accurate
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

// 0 - 1000
// curr = 0: (0 - 200)
// curr = 200: (0 - 400)
// curr = 500: (400-600)

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
void Scout::moveUnit() {
	// We start with an empty path
	//std::cout << "path size " << currentPath.size() << std::endl;

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

void Worker::calculateNewPath() {
	//int randomNodeIdx = getRandomNumber(0, mapReference->accessableTiles->scoutedPaths.size() - 1);

	//currentTileIdx = getcurrentCorrespondingTile(mapReference->accessableTiles->scoutedPaths);

	//currentPath = mapReference->accessableTiles->AStar(
	//	mapReference->accessableTiles->scoutedPaths[currentTileIdx],
	//	mapReference->accessableTiles->scoutedPaths[randomNodeIdx],
	//	mapReference->accessableTiles->scoutedPaths,
	//	mapReference->accessableTiles->ScoutedPathsNeighboors);

	//UnitBase::calculateNewPath();
}

void Worker::moveUnit() {
	// We start with an empty path
	//std::cout << "path size: " << currentPath.size() << std::endl;

	//int scoutPathCount = mapReference->accessableTiles->scoutedPaths.size();
	//string stringThing = "Scouted Path count: " + to_string(scoutPathCount);
	//const char* charThing = stringThing.c_str();
	//DrawText(charThing, 20, 1150, 24, PURPLE);

	//auto ref = mapReference->accessableTiles;
	//if (currentPath.size() == 0 && ref->scoutedPaths.size() >= 1) {
	//	AwaitNewPath();
	//}
	//else {
	//	/// Casually move toward targetPos
	//	if (Vector2Distance(pos, targetPos) > 10) {
	//		moveUnitTowardsInternalGoal();
	//	}
	//	// we hit our next goal
	//	if (Vector2Distance(pos, targetPos) < 5) {
	//		if (connectionIdx + 1 < currentPath.size()) {
	//			targetPos.x = (float)currentPath[connectionIdx].toNode.x;
	//			targetPos.y = (float)currentPath[connectionIdx].toNode.y;
	//			connectionIdx++;
	//		}

	//	}
	//	// we reached the end of our path, so therefore reset and make new path
	//	if (connectionIdx >= currentPath.size() - 1) {
	//		currentPath.clear();
	//		connectionIdx = 0;
	//		std::cout << "Path ended" << std::endl;
	//	}
	//}
}

