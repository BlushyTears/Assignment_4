#pragma once

#include <iostream>
#include <vector>
#include "Units.h"

#include "raylib.h"
#include "GameManager.h"
#include "MapManager.h"
#include "TaskManager.h"
#include "Timer.h"

struct ResourceTracker;
struct Timer;

struct Building {
	Vector2 pos;
	Timer timer;

	virtual void update() = 0;
};

struct CoalMile : Building {
	ResourceTracker* resourceTracker;
	int treeCount = 0;
	int minTreesNeeded = 10;
	int coalCount = 0;
	bool isActive = false;

	CoalMile(Vector2 _pos, ResourceTracker* _rt) {
		pos = _pos;
		resourceTracker = _rt;
	}

	void update() override {
		if (!timer.hasTimerEnded()) {
			timer.updateTimer();
			return;
		}
		else if (timer.hasTimerEnded()) {
			// if we aren't smelting (This should be guaranteed to run once)
			if (!isActive && treeCount >= minTreesNeeded) {
				isActive = true;
				timer.setNewTimer(6);
			}
			// if we are done smelting our current batch
			else if(isActive) {
				treeCount -= 10;
				coalCount++;
				isActive = false;
			}
		}
	}
};