#include "Building.h"


void CoalMile::update() {
    debugText();
    if (!isBuilt)
        return;
    if (isActive) {
        produceTimer.updateTimer();
        if (produceTimer.hasTimerEnded()) {
            resourceTracker->coalCount++;
            resourceTracker->treeCount -= costPerCoal;
            coalCount++;
            isActive = false;
        }
    }
    else if (treeCount >= costPerCoal) {
        isActive = true;
        treeCount -= costPerCoal;
        resourceTracker->woodInCoalMile = treeCount;
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
            if (resourceTracker->ironArrowCount < ironArrowsGoal) { 
                ironArrowCount++;
                resourceTracker->ironArrowCount++;
                isActive = false;
            }
            else {
                ironBarCount++;
                resourceTracker->ironBarCount++;
                isActive = false;
            }
        }
    }
    else if (ironOreCount >= 2 && coalCount >= 3 && resourceTracker->ironArrowCount < ironArrowsGoal) {
        isActive = true;
        ironOreCount -= 2;
        coalCount -= 3;
        produceTimer.setNewTimer(6);
    }
    else if (ironOreCount >= 2 && coalCount >= 3) {
        isActive = true;
        ironOreCount -= 2;
        coalCount -= 3;
        produceTimer.setNewTimer(4);
    }
}

void ArmSmith::update() {
    debugText();
    if (!isBuilt)
        return;
    if (isActive) {
        produceTimer.updateTimer();
        if (produceTimer.hasTimerEnded()) {
            ironSwordCount++;
            isActive = false;
        }
    }
    else if (coalCount >= coalNeeded && ironBarCount >= ironBarNeeded) {
        isActive = true;
        coalCount -= coalNeeded;
        ironBarCount -= ironBarNeeded;
        produceTimer.setNewTimer(6);
    }
}

void TrainingCamp::update() {
    debugText();
    if (!isBuilt)
        return;
    if (isActive) {
        produceTimer.updateTimer();
        if (produceTimer.hasTimerEnded()) {
            isActive = false;
        }
    }
    else if (swordCount >= swordsNeeded && isWorkerAvailable) {
        isActive = true;
        swordCount -= swordsNeeded;
        resourceTracker->ironSwordCount++;
        produceTimer.setNewTimer(6);
    }
}