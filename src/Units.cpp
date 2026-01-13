#include "Units.h"
#include "MapManager.h"

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
		if (tile.isUnitWithinTile(*this, 10) && !tile.hasBeenScouted) {
			tile.hasBeenScouted = true;
		}
	}
}