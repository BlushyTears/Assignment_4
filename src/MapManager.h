#pragma once

#include "raylib.h"
#include "resource_dir.h"	

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "GraphMap.h"
#include "Units.h"
#include "Worker.h"

struct UnitBase;
struct Worker;
struct ResourceTracker;

using namespace std;

enum TileType {
	Trees, // T
	Water, // V
	Swamp, // G
	Rock,  // B
	Grass, // M
};

enum EntityTypes {
	eNone,
	eTree,
	eFelledTree,
	eCoal,
	eIronOre,
	eIronBar
};

struct Entity {
	int idx;
	EntityTypes entityType;
	Vector2 tileOffset; // dont spawn all trees on same pixel pos etc (If it's iron ore we default to 0,0)
	Color entityColor;
	bool reserved = false;

	Entity() {}

	Entity(int _c, EntityTypes _et, Vector2 _pos, Color _ec) {
		idx = _c;
		entityType = _et;
		tileOffset = _pos;
		entityColor = _ec;
	}

	bool operator ==(const Entity& other) const {
		return idx == other.idx;
	}

	bool operator ==(int targetIdx) const {
		return idx == targetIdx;
	}
};

struct Tile {
	Vector2 position;
	bool hasBeenScouted = false;

	TileType tileType;
	Color tileColor;

	std::vector<Entity> occupyingEntities;

	Tile(int _x, int _y) {
		position.x = _x;
		position.y = _y;
	}

	// In case the tile has entities on it
	void spawnTrees(int _tileSize) {
		if (tileType == Trees) {
			int treeCount = 5;
			for (int i = 0; i < treeCount; i++) {
				int x = getRandomNumber(1, _tileSize);
				int y = getRandomNumber(1, _tileSize);

				Entity entity(i, eTree, Vector2{ (float)x, (float)y }, Color{100, 200, 25, 255});
				occupyingEntities.push_back(entity);
			}
		}
	}
};

struct Map {
	using MapIndex = int;

	std::vector<Tile> renderedTiles; // map
	ChartedMap* accessableTiles = nullptr;
	ChartedMap* scoutedTiles = nullptr;

	std::vector<MapIndex> scoutedTreeTileIndices; // Future tech for finding trees without looping through all tiles
	std::vector<MapIndex> ironOreIndices; // Future tech for finding ores without looping through all tiles
	std::queue<UnitBase*> searchQueue; // Units that want to search the map currently.
	int mapTileSize;

	Vector2 getNearestTreePos(Worker& unit);
	// find closest scouted tree relative to unit and chop that down
	bool fellTree(Worker& unit);

	//int getNearestOreIdx(UnitBase& unit) {
	//}
	// Todo: singular tree only
	void removeTreeByIndex(int _treeTileTargetIdx, int _treeTargetIdx);

	Map(stringstream& ss, int _screenWidth, int _tileSize) {
		accessableTiles = new ChartedMap(ss.str(), _tileSize);
		mapTileSize = _tileSize;
		string line;
		int row = 0;
		
		while (getline(ss, line)) {
			for (int col = 0; col < (int)line.length(); col++) {
				Tile tempTile(col * _tileSize, row * _tileSize);
				int index = row * (int)line.length() + col;
				if (line[col] == 'T') {
					tempTile.tileColor = { 50, 255, 50, 100 };
					tempTile.tileType = Trees;
					tempTile.spawnTrees(mapTileSize);
				}
				else if (line[col] == 'V') {
					tempTile.tileColor = { 50, 50, 255, 255 };
					tempTile.tileType = Water;
				}
				else if (line[col] == 'G') {
					tempTile.tileColor = { 150, 235, 10, 255 };
					tempTile.tileType = Swamp;
				}
				else if (line[col] == 'B') {
					tempTile.tileColor = { 50, 50, 50, 255 };
					tempTile.tileType = Rock;
				}
				else if (line[col] == 'M') {
					tempTile.tileColor = { 30, 255, 20, 255 };
					tempTile.tileType = Grass;
				}
				renderedTiles.push_back(tempTile);
			}
			row++;
		}

		scoutedTiles = new ChartedMap(_tileSize);
		spawnIronOre();
	}
	void spawnIronOre() {
		for (int i = 0; i < 60; i++) {
			bool canSpawnHere = false;
			int randomTileIdx = 0;
			while (!canSpawnHere) {
				randomTileIdx = GetRandomValue(0, renderedTiles.size() - 1);

				// make sure our iron ore doesn't spawn in areas we can't access
				// aka only spawn on grass and swamp
				if (renderedTiles[randomTileIdx].hasBeenScouted
					|| renderedTiles[randomTileIdx].tileType == Water
					|| renderedTiles[randomTileIdx].tileType == Rock
					|| renderedTiles[randomTileIdx].tileType == Trees) {
					continue;
				}
				else {
					canSpawnHere = true;
				}
			}

			if (canSpawnHere) {
				int entityCount = 1; // This should be read externally
				Entity entity(entityCount, eIronOre, {0, 0}, Color{ 10, 10, 10, 255 });
				renderedTiles[randomTileIdx].occupyingEntities.push_back(entity);
				ironOreIndices.push_back(randomTileIdx);
			}
		}
	}

	void renderMap(int _screenWidth, int _screenHeight, int _tileSize) {
		// make this const reference whenever other thing runs im working on
		for (auto const& tile : renderedTiles) {
			if (!tile.hasBeenScouted) {
				DrawRectangle(tile.position.x, tile.position.y, _tileSize, _tileSize, GRAY);
			}
			else {
				DrawRectangle(tile.position.x, tile.position.y, _tileSize, _tileSize, tile.tileColor);
				for (auto& entity : tile.occupyingEntities) {
					DrawRectangle(	tile.position.x + entity.tileOffset.x,
									tile.position.y + entity.tileOffset.y,
									_tileSize / 8, _tileSize / 8, { entity.entityColor });
				}
			}
		}
	}
};

inline stringstream transcribeData(const string& _path) {
	ifstream file(_path);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer;
}
