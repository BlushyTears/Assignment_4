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
		}
	}
}
void Scout::moveUnit() {
	Vector2 start = { pos.x, pos.y };

	if (Vector2Distance(pos, targetPos) > 10) {
		moveUnitTowardsInternalGoal();
		testTile();
	}
	else {
		int randomNodeIdx = getRandomNumber(0, mapReference->accessableTiles->walkablePaths.size() - 1);
		std::cout << randomNodeIdx << " Is the random number currently" << std::endl;
		Vector2 randomNode = mapReference->accessableTiles->walkablePaths[randomNodeIdx];
		targetPos = { randomNode.x, randomNode.y };
	}
}

void Worker::moveUnit() {
	//Vector2 start = { pos.x, pos.y };

	//auto lastTile = (*renderedTiles)[renderedTiles->size() - 1];
	//Vector2 goal = { lastTile.positionX, lastTile.positionY };
}