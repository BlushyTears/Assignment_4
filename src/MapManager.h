#pragma once

#include "raylib.h"
#include "resource_dir.h"	

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector> // probably should replace this 

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

	TileType tileType = Grass;
	Color tileColor;

	Tile(int _x, int _y) {
		positionX = _x;
		positionY = _y;
	}
};

struct Map {
	std::vector<Tile> tiles;
	Map(stringstream& ss, int _screenWidth, int _tileSize) {
		tiles.reserve(10000);

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
				tiles.push_back(tempTile);
			}
			row++;
		}
	}

	void renderMap(int _screenWidth, int _screenHeight, int _tileSize) {
		for (auto tile : tiles) {
			DrawRectangle(tile.positionX, tile.positionY, _tileSize, _tileSize, tile.tileColor);
		}
	}
};

stringstream transcribeData(const string& _path) {
	ifstream file(_path);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer;
}
