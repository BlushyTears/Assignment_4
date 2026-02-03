#include "Building.h"


void CoalMile::update() {
	debugText();

	materialsAvailable = (treeCount >= minTreesNeeded);

	if (!isBuilt) return;

	if (isActive) {
		produceTimer.updateTimer();
		if (produceTimer.hasTimerEnded()) {
			resourceTracker->treeCount--;
			coalCount++;
			isActive = false;
		}
	}
	else if (treeCount >= minTreesNeeded) {
		isActive = true;
		treeCount -= costPerCoal;
		produceTimer.setNewTimer(6);
	}
}

void Smelter::update() {
	debugText();

	materialsAvailable = (treeCount >= minTreesNeeded);

	if (!isBuilt) return;

	if (isActive) {
		produceTimer.updateTimer();
		if (produceTimer.hasTimerEnded()) {
			resourceTracker->treeCount--;
			ironCount++;
			isActive = false;
		}
	}
	else if (treeCount >= minTreesNeeded) {
		isActive = true;
		treeCount -= costPerIron;
		produceTimer.setNewTimer(6);
	}
}