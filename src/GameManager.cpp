#include "GameManager.h"

Game::Game(int _initialUnits) {
	mapData = transcribeData("..//mapData.txt");
	gameMap = new Map(mapData, SCREEN_WIDTH, TILE_SIZE);
	targetResourceCount = new ResourceTracker();
	actualResourceCount = new ResourceTracker();

	for (int i = 0; i < _initialUnits; i++) {
		if (xCount % initialFormationColumns == 0) {
			xCount = 1;
			yCount++;
		}

		units.push_back(std::make_unique<Worker>(
			baseXUnitSpawn + (spacing * xCount),
			baseYUnitSpawn + (spacing * yCount),
			gameMap,
			targetResourceCount,
			&units,
			gameMap->buildings));
		xCount++;

		units[i]->testTile();
	}
}

void Game::startTrainingUnits(UnitToTrain unitType) {
	// We're done training units
	if (unitType == EnumNone) {
		return;
	}
	for (auto& unit : units) {
		Worker* worker = dynamic_cast<Worker*>(unit.get());

		float trainTime = 5.0f;
		switch (unitType) {
			case EnumScout: trainTime = 3.0f; break;
			case EnumCoalMiner: trainTime = 3.0f; break;
			case EnumBuilder: trainTime = 3.0f; break;
		}

		if (worker && !worker->isTraining) {
			worker->isTraining = true;
			Event trainEvent(worker, unitType, trainTime);
			trainingUnits.push_back(trainEvent);
			break;
		}
	}
}

UnitToTrain Game::getNextUnitToTrain() {
	if (targetResourceCount->scoutCount < 8) {
		targetResourceCount->scoutCount++;
		return EnumScout;
	}
	else if (targetResourceCount->coalMinerCount < 1) {
		targetResourceCount->coalMinerCount++;
		return EnumCoalMiner;
	}
	else if (targetResourceCount->builderCount < 1) {
		targetResourceCount->builderCount++;
		return EnumBuilder;
	}
	return EnumNone;
}

void Game::callUnits() {
	int idx = 0;
	for (auto& unit : units) {
		idx++;
		unit->renderUnit();

		if (!unit->isTraining) {
			unit->commandUnit();
			unit->moveUnitTowardsInternalGoal();
		}
	}

	if (!gameMap->searchQueue.empty()) {
		UnitBase* unit = gameMap->searchQueue.front();
		gameMap->searchQueue.pop();
		unit->calculateNewPath();
	}
}

void Game::update() {
	debugText();

	if (trainingUnits.size() < 3) {
		UnitToTrain nextUnit = getNextUnitToTrain();
		startTrainingUnits(nextUnit);
	}

	gameMap->renderMap(SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE);
	callUnits();
	updateTrainingUnits();
	controlBuildings();
	gameMap->drawBuildings();
}

void Game::convertUnit(UnitBase* unitPtr, UnitToTrain unitType) {
	for (auto& unit : units) {
		if (unit.get() == unitPtr) {
			Vector2 tempPos = unitPtr->pos;
			actualResourceCount->workerCount--;

			switch (unitType) {
			case EnumScout:
				unit = std::make_unique<Scout>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				actualResourceCount->scoutCount++;
				break;
			case EnumCoalMiner:
				unit = std::make_unique<CoalWorker>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				actualResourceCount->coalMinerCount++;
				break;
			case EnumBuilder:
				unit = std::make_unique<Builder>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				actualResourceCount->builderCount++;
				break;
			case EnumNone:
				break;
			}
			unit->isTraining = false;
			break;
		}
	}
}

// Idea: Spawn buildings once we have dicovered enough terrain to practically place them close to each other with some variety
void Game::spawnBuildings() {
	if (gameMap->scoutedTiles->walkablePaths.size() < 100)
		return;

	bool hasFoundGoodLand = false;

	while (!hasFoundGoodLand) {
		int randomNodeIdx = getRandomNumber(0, (gameMap->renderedTiles.size() - 1));
		Tile& tile = gameMap->renderedTiles[randomNodeIdx];

		if (!tile.hasBeenScouted || tile.tileType != Grass)
			continue;

		bool occupied = false;
		for (auto* b : gameMap->buildings) {
			if (Vector2Distance(tile.position, b->pos) < TILE_SIZE) {
				occupied = true;
				break;
			}
		}

		if (occupied)
			continue;

		hasFoundGoodLand = true;
		if (targetResourceCount->coalCount == 0) {
			CoalMile* cm = new CoalMile({ tile.position.x, tile.position.y }, targetResourceCount, TILE_SIZE);
			gameMap->buildings.push_back(cm);
			targetResourceCount->coalCount++;
		}
		else if (targetResourceCount->smelterBuildingCount == 0) {
			Smelter* s = new Smelter({ tile.position.x, tile.position.y }, targetResourceCount, TILE_SIZE);
			gameMap->buildings.push_back(s);
			targetResourceCount->smelterBuildingCount++;
		}
		else {
			return;
		}
	}
}

void Game::updateTrainingUnits() {
	for (auto it = trainingUnits.begin(); it != trainingUnits.end();) {
		it->trainTimer.updateTimer();

		if (it->trainTimer.hasTimerEnded()) {
			convertUnit(it->unitPtr, it->unitToTrain);
			it = trainingUnits.erase(it);
		}
		else {
			it++;
		}
	}
}

void Game::controlBuildings() {
	this->spawnBuildings();
	for (auto building : gameMap->buildings) {
		building->update();
	}
}