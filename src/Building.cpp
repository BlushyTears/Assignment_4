#include "Building.h"


void CoalMile::update() {
	debugText();

	if (!isBuilt) 
		return;

	if (isActive) {
		produceTimer.updateTimer();
		if (produceTimer.hasTimerEnded()) {
			resourceTracker->treeCount--;
			coalCount++;
			isActive = false;
		}
	}
	else if (treeCount >= costPerCoal) {
		isActive = true;
		treeCount -= costPerCoal;
		produceTimer.setNewTimer(6);
	}
}

void Smelter::update() {
	debugText();

	if (!isBuilt) 
		return;

	if (isActive) {
		produceTimer.updateTimer();
		if (produceTimer.hasTimerEnded()) {
			ironBarCount++;
			resourceTracker->ironBarCount++;
			isActive = false;
		}
	}
	else if (ironOreCount >= 2 && coalCount >= 3) {
		isActive = true;
		ironOreCount -= 2;
		coalCount -= 3;
		produceTimer.setNewTimer(6);
	}
}