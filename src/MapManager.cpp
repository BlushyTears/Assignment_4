#include "MapManager.h"

bool Tile::isUnitWithinTile(const UnitBase& unit, int perimiterCheck) {
	if (unit.pos.x >= positionX && unit.pos.x < positionX + perimiterCheck &&
		unit.pos.y >= positionY && unit.pos.y < positionY + perimiterCheck) {
		return true;
	}
	return false;
}