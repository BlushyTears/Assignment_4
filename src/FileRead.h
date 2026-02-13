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
		}
	}
}
