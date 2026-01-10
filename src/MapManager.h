#pragma once

#include "raylib.h"
#include "resource_dir.h"	

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector> // probably should replace this 

using namespace std;

struct Tile {
	int positionX;
	int positionY;

	Tile(int _x, int _y) {
		positionX = _x;
		positionY = _y;
	}
};

struct Map {
	std::vector<Tile> tiles;
	Map(const string& _mapData, int _screenWidth, int _tileSize) {
		tiles.reserve(10000);

		int rowCount = 0;
		for (int i = 0; i < _mapData.size(); i++) {
			for (int j = 0; j < _screenWidth / _tileSize; j++) {
				Tile tempTile = Tile(i, j);
				tiles.push_back(tempTile);
			}
		}
	}

	void renderMap(int _screenWidth, int _screenHeight) {
		for (int i = 0; i < _screenWidth; i++) {
			for (int j = 0; j < _screenHeight; j++) {
				DrawPixel(i, j, RED);
			}
		}
	}
};

string translateMapToString(const string& _path) {
	ifstream file(_path);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
