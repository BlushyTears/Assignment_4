#pragma once

#include "raylib.h"
#include "resource_dir.h"	

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "GraphMap.h"
#include "Units.h"

struct UnitBase;

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
	int count;

	EntityTypes entityType;
	Vector2 tileOffset; // dont spawn all trees on same pixel pos etc (If it's iron ore we default to 0,0)
	Color entityColor;

	Entity() {}

	Entity(int _c, EntityTypes _et, Vector2 _pos, Color _ec) {
		count = _c;
		entityType = _et;
		tileOffset = _pos;
		entityColor = _ec;
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

				Entity entity(treeCount, eTree, Vector2{ (float)x, (float)y }, Color{100, 200, 25, 255});
				occupyingEntities.push_back(entity);
			}
		}
	}
	bool isUnitWithinTile(const UnitBase& unit, int perimiterCheck);
};

struct Map {
	std::vector<Tile> renderedTiles;
	ChartedMap* accessableTiles; // TODO: Make this a smart pointer
	//std::vector<Connection> thing = ChartedMap.AStar();

	std::vector<int> ironOreIndices; // Future tech for finding ores without looping through all tiles

	int mapTileSize;

	Map(stringstream& ss, int _screenWidth, int _tileSize) {
		accessableTiles = new ChartedMap(ss.str(), _tileSize);
		mapTileSize = _tileSize;
		string line;
		int row = 0;

		while (getline(ss, line)) {
			for (int col = 0; col < (int)line.length(); col++) {
				Tile tempTile(col * _tileSize, row * _tileSize);

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
					tempTile.tileColor = { 10, 235, 10, 255 };
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
				Entity entity(entityCount, eIronOre, {0, 0}, Color{ 255, 255, 255, 255 });
				renderedTiles[randomTileIdx].occupyingEntities.push_back(entity);
				ironOreIndices.push_back(randomTileIdx);
			}
		}
	}

	// Called sequentially to reduce overhead even if it limits the possible tiles some
	void updateScoutedMapData() {
		if (accessableTiles->scoutedPaths.size() % 100 == 0) {
			//std::cout << "Update neighboors, rendered tiles: " << accessableTiles->scoutedPaths.size() << std::endl;
			accessableTiles->computeNeighboors(accessableTiles->scoutedPaths, accessableTiles->ScoutedPathsNeighboors);
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
									_tileSize / 2, _tileSize / 2, { entity.entityColor });
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
