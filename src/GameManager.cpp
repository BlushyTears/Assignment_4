#include "GameManager.h"
#include "FileRead.h"

Game::Game() {
	setupStartingValues();

	mapData = transcribeData("..//mapData.txt");
	gameMap = new Map(mapData, SCREEN_WIDTH, TILE_SIZE);
	targetResourceCount = new ResourceTracker();
	actualResourceCount = new ResourceTracker();

	targetResourceCount->workerCount = resourceParameters.workerCount;

	for (int i = 0; i < resourceParameters.workerCount; i++) {
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

		float trainTime = 5.0;
		switch (unitType) {
		case EnumScout: trainTime = (resourceParameters.scoutTrainTime / resourceParameters.timeControl); break;
			case EnumCoalMiner: trainTime = (resourceParameters.craftsmanTrainTime / resourceParameters.timeControl); break;
			case EnumBuilder: trainTime = (resourceParameters.craftsmanTrainTime / resourceParameters.timeControl); break;
			case EnumSoldier: trainTime = (resourceParameters.soldierTrainTime / resourceParameters.timeControl);break;
		}

		if (worker && !worker->isTraining) {
			if (unitType != EnumSoldier || (unit->isReadyToBeSoldier && worker->trainingCamp->swordCount > 0)) {
				if (unitType == EnumSoldier) {
					targetResourceCount->ironSwordCount--;
					worker->trainingCamp->swordCount--;
				}
				
				worker->isTraining = true;
				Event trainEvent(worker, unitType, trainTime);
				trainingUnits.push_back(trainEvent);
				break;
			}
		}
	}
}

UnitToTrain Game::getNextUnitToTrain() {
	if (targetResourceCount->scoutCount < 12) {
		targetResourceCount->scoutCount++;
		return EnumScout;
	}
	else if (targetResourceCount->builderCount < 1) {
		targetResourceCount->builderCount++;
		return EnumBuilder;
	}
	else if (targetResourceCount->coalMinerCount < 1 && targetResourceCount->coalMileCount >= 0) {
		targetResourceCount->coalMinerCount++;
		return EnumCoalMiner;
	}
	else if (targetResourceCount->armSmithCount < 1 && targetResourceCount->armsmithForgeCount >= 0) {
		targetResourceCount->armSmithCount++;
		return EnumArmSmith;
	}
	else if (targetResourceCount->smelterCount < 1 && targetResourceCount->smelterBuildingCount >= 0) {
		targetResourceCount->smelterCount++;
		return EnumSmelter;
	}
	else if (targetResourceCount->ironSwordCount > 0 && targetResourceCount->soldierCount <= 20) {
		return EnumSoldier;
	}
	return EnumNone;
}

void Game::callUnits() {
	for (auto& unit : units) {
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

	if (IsKeyPressed(KEY_UP)) {
		resourceParameters.timeControl += 1;
	}
	if (IsKeyPressed(KEY_DOWN)) {
		resourceParameters.timeControl -= 1;
	}

	UnitToTrain nextUnit = getNextUnitToTrain();
	startTrainingUnits(nextUnit);

	gameMap->renderMap(SCREEN_WIDTH, SCREEN_HEIGHT, TILE_SIZE);
	callUnits();
	updateTrainingUnits();
	controlBuildings();
	gameMap->drawBuildings();

	std::string text = "Speed Multiplier " + to_string(resourceParameters.timeControl);;
	DrawText(text.c_str(), (SCREEN_WIDTH / 2) - 150, SCREEN_HEIGHT / 8, 24, GOLD);
}

void Game::convertUnit(UnitBase* unitPtr, UnitToTrain& unitType) {
	for (auto& unit : units) {
		if (unit.get() == unitPtr) {

			Vector2 tempPos = unit->pos;
			targetResourceCount->workerCount--;

			switch (unitType) {
			case EnumScout:
				unit = std::make_unique<Scout>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				break;
			case EnumCoalMiner:
				unit = std::make_unique<CoalWorker>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				break;
			case EnumBuilder:
				unit = std::make_unique<Builder>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				break;
			case EnumSmelter:
				unit = std::make_unique<SmelterWorker>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				break;
			case EnumArmSmith:
				unit = std::make_unique<ArmSmithWorker>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				break;
			case EnumSoldier:
				unit = std::make_unique<Soldier>(tempPos.x, tempPos.y, gameMap, targetResourceCount, &units, gameMap->buildings);
				targetResourceCount->soldierCount++;
				/*targetResourceCount->ironSwordCount--;*/
				std::cout << "Actually trained a soldier pos x:" << tempPos.x << " pos y: " << tempPos.y << std::endl;
				break;
			case EnumNone:
				break;
			}

			unit->isTraining = false;
		}
	}
}

// Implenetation details: We first spawn a "schematic" of a building which serves the purpose of being a place where units can put materials
// But the building isn't built yet, so it won't have any functional purpose beyond.
void Game::spawnBuildings() {
	if (gameMap->scoutedTiles->walkablePaths.size() < 120)
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
		else if (targetResourceCount->armsmithForgeCount == 0) {
			ArmSmith* s = new ArmSmith({ tile.position.x, tile.position.y }, targetResourceCount, TILE_SIZE);
			gameMap->buildings.push_back(s);
			targetResourceCount->armsmithForgeCount++;
		}
		else if (targetResourceCount->trainingCamp == 0) {
			TrainingCamp* s = new TrainingCamp({ tile.position.x, tile.position.y }, targetResourceCount, TILE_SIZE);
			tc = s;
			gameMap->buildings.push_back(s);
			targetResourceCount->trainingCamp++;
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