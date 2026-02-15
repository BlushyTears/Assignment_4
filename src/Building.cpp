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
            this->coalCount++;
            isActive = false;
        }
    }
    else if (treeCount >= costPerCoal && coalCount < coalCountCap) {
        isActive = true;
        treeCount -= costPerCoal;
        resourceTracker->woodInCoalMile = treeCount;
        produceTimer.setNewTimer(resourceParameters.coalProductionTimer / resourceParameters.timeControl);
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
    else if (ironOreCount >= ironOreCostPerIronBar && coalCount >= coalCostPerIronBar && resourceTracker->ironArrowCount < ironArrowsGoal) {
        isActive = true;
        ironOreCount -= ironOreCostPerIronBar;
        coalCount -= coalCostPerIronBar;
        produceTimer.setNewTimer(resourceParameters.ironBarProductionTimer / resourceParameters.timeControl);
    }
    else if (ironOreCount >= ironOreCostPerIronBar && coalCount >= coalCostPerIronBar) {
        isActive = true;
        ironOreCount -= ironOreCostPerIronBar;
        coalCount -= coalCostPerIronBar;
        produceTimer.setNewTimer(resourceParameters.ironBarProductionTimer / resourceParameters.timeControl);
    }
}

void ArmSmith::update() {
    debugText();
    if (!isBuilt)
        return;
    if (isActive) {
        produceTimer.updateTimer();
        if (produceTimer.hasTimerEnded()) {
            resourceTracker->ironSwordCount++;
            ironSwordCount++;
            isActive = false;
        }
    }
    else if (coalCount >= coalNeeded && ironBarCount >= ironBarNeeded) {
        isActive = true;
        coalCount -= coalNeeded;
        ironBarCount -= ironBarNeeded;
        produceTimer.setNewTimer(resourceParameters.swordProductionTimer / resourceParameters.timeControl);
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
            isWorkerAvailable = false;
        }
    }
    else if (swordCount >= swordsNeeded && isWorkerAvailable) {
        // Comment: resourceTracker->soldierCount is incremented in game manager
        std::cout << "Training soldier, swords at training camp: " << swordCount << std::endl;
        isActive = true;
        //swordCount -= swordsNeeded;
        produceTimer.setNewTimer(resourceParameters.soldierTrainTime / resourceParameters.timeControl);
    }
}