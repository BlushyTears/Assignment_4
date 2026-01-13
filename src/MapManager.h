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

struct Tile {
	int positionX;
	int positionY;
	bool hasBeenScouted = false;

	TileType tileType = Grass;
	Color tileColor;

	Tile(int _x, int _y) {
		positionX = _x;
		positionY = _y;
	}
	bool isUnitWithinTile(const UnitBase& unit, int perimiterCheck);
};

struct Map {
	std::vector<Tile> renderedTiles;
	ChartedMap* accessableTiles; // TODO: Make this a smart pointer
	//std::vector<Connection> thing = ChartedMap.AStar();

	// draw path example (dont remove):
	//for (int i = 0; i < shortestPath.size(); i++) {
	//    DrawCircle( shortestPath[i].toNode.x + tileSize / 2, shortestPath[i].toNode.y + tileSize / 2 , 5 / 2, GREEN);
	//}

	Map(stringstream& ss, int _screenWidth, int _tileSize) {
		renderedTiles.reserve(10000);
		accessableTiles = new ChartedMap(ss.str(), _tileSize);

		string line;
		int row = 0;
		while (getline(ss, line)) {
			for (int col = 0; col < (int)line.length(); col++) {
				Tile tempTile(col * _tileSize, row * _tileSize);
				if (line[col] == 'T') {
					tempTile.tileColor = { 50, 255, 50, 100 };
				}
				else if (line[col] == 'V') {
					tempTile.tileColor = { 50, 50, 255, 255 };
				}
				else if (line[col] == 'G') {
					tempTile.tileColor = { 255, 50, 100, 255 };
				}
				else if (line[col] == 'B') {
					tempTile.tileColor = { 50, 50, 50, 255 };
				}
				else if (line[col] == 'M') {
					tempTile.tileColor = { 30, 255, 20, 255 };
				}
				renderedTiles.push_back(tempTile);
			}
			row++;
		}
	}

	void renderMap(int _screenWidth, int _screenHeight, int _tileSize) {
		// make this const reference whenever other thing runs im working on
		for (auto const& tile : renderedTiles) {
			if (!tile.hasBeenScouted) {
				DrawRectangle(tile.positionX, tile.positionY, _tileSize, _tileSize, GRAY);
			}
			else {
				DrawRectangle(tile.positionX, tile.positionY, _tileSize, _tileSize, tile.tileColor);
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
