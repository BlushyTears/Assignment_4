#pragma once

#include <fstream>
#include <string>
#include <iostream>

// Values specified in design document stored here (With default values as safety precaution)
struct ResourceParams {
	float grassMovementSpeed;
	float swampMovementSpeed;
	float timeControl;
	float treesPerTile;
	float workerCount;
	float scoutTrainTime;
	float soldierTrainTime;
	float craftsmanTrainTime;
	float treeChopDownTimer;
	float coalProductionTimer;
	float ironBarProductionTimer;
	float swordProductionTimer;
	float coalMileBuildTime;
	float SmelterBuildTime;
	float ArmsmithBuildTime;
	float TrainingCampBuildTime;
};

inline ResourceParams resourceParameters;

inline void setupStartingValues() {
	std::fstream coldData("..//DesignValues.txt");

	std::string line;
	while (std::getline(coldData, line)) {

		size_t delimPos = line.find(":");
		bool foundDelim = (delimPos < line.length());

		if(foundDelim) {
			std::string key = line.substr(0, delimPos);

			std::string valueStr = line.substr(delimPos + 1);

			float value = std::stof(valueStr);

			if (key == "timeControl") {
				resourceParameters.timeControl = value;
			}
			if (key == "grassMovementSpeed") {
				resourceParameters.grassMovementSpeed = value;
			}
			if (key == "swampMovementSpeed") {
				resourceParameters.swampMovementSpeed = value;
			}
			if (key == "treesPerTile") {
				resourceParameters.treesPerTile = value;
			}
			if (key == "workerCount") {
				resourceParameters.workerCount = value;
			}
			if (key == "scoutTrainTime") {
				resourceParameters.scoutTrainTime = value;
			}
			if (key == "soldierTrainTime") {
				resourceParameters.soldierTrainTime = value;
			}
			if (key == "craftsmanTrainTime") {
				resourceParameters.craftsmanTrainTime = value;
			}
			if (key == "treeChopDownTimer") {
				resourceParameters.treeChopDownTimer = value;
			}
			if (key == "coalProductionTimer") {
				resourceParameters.coalProductionTimer = value;
			}
			if (key == "ironBarProductionTimer") {
				resourceParameters.ironBarProductionTimer = value;
			}
			if (key == "swordProductionTimer") {
				resourceParameters.swordProductionTimer = value;
			}
			if (key == "coalMileBuildTime") {
				resourceParameters.coalMileBuildTime = value;
			}
			if (key == "SmelterBuildTime") {
				resourceParameters.SmelterBuildTime = value;
			}
			if (key == "ArmsmithBuildTime") {
				resourceParameters.ArmsmithBuildTime = value;
			}
			if (key == "TrainingCampBuildTime") {
				resourceParameters.TrainingCampBuildTime = value;
			}
		}
	}
}
