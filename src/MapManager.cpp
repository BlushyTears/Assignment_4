#include "MapManager.h"

bool Tile::isUnitWithinTile(const UnitBase& unit, int perimiterCheck) {
	if (unit.pos.x >= position.x && unit.pos.x < position.x + perimiterCheck &&
		unit.pos.y >= position.y && unit.pos.y < position.y + perimiterCheck) {
		return true;
	}
	return false;
}