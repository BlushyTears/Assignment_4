#pragma once

#include "raylib.h"
//#include "resource_dir.h"

#include <iostream>

struct UnitBase {
	int x;
	int y;
	// All units should be equally big so it's hard coded here
	int size = 3;

	UnitBase(int _x, int _y) : x(_x), y(_y) {}
	virtual void renderWorker() = 0;
};

struct Worker : UnitBase {
	Worker(int _x, int _y) : UnitBase(_x, _y) {}
	void renderWorker() {
		DrawCircle(x, y, size, RED);
	}
};
