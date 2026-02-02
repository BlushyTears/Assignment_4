#include "MapManager.h"

bool Map::tryToFellTree(Worker& unit) {
    if (unit.treeTileTargetIdx == -1 || unit.treeTargetIdx == -1) 
        return false;
    if (unit.isChoppingWood && !unit.chopTimer.hasTimerEnded()) 
        return false;
    if (unit.isChoppingWood && unit.chopTimer.hasTimerEnded()) {
        unit.isChoppingWood = false;
        return true;
    }

    auto& entities = renderedTiles[unit.treeTileTargetIdx].occupyingEntities;
    for (auto& entity : entities) {
        if (entity.idx == unit.treeTargetIdx && entity.entityType == eTree) {
            Vector2 treePos = renderedTiles[unit.treeTileTargetIdx].position + entity.tileOffset;
            if (Vector2Length(unit.pos - treePos) <= 12.5f) {
                unit.isChoppingWood = true;
                unit.chopTimer.setNewTimer(3);
            }
            return false;
        }
    }
    return false;
}

Vector2 Map::getNearestTreePos(Worker& unit) {
    // sanity check
    if (unit.treeTileTargetIdx != -1 && unit.treeTargetIdx != -1) {
        for (auto& entity : renderedTiles[unit.treeTileTargetIdx].occupyingEntities) {
            if (entity.idx == unit.treeTargetIdx && entity.entityType == eTree) {
                entity.reserved = false;
                break;
            }
        }
    }

    float minTreeDist = std::numeric_limits<float>::max();
    Entity* nearestTree = nullptr;
    int tile = -1;

    for (int treeTileIdx : scoutedTreeTileIndices) {
        for (auto& entity : renderedTiles[treeTileIdx].occupyingEntities) {
            if (entity.entityType != eTree || entity.reserved) 
                continue;
            float dist = Vector2Length((renderedTiles[treeTileIdx].position + entity.tileOffset) - unit.pos);
            if (dist < minTreeDist) {
                minTreeDist = dist;
                nearestTree = &entity;
                tile = treeTileIdx;
            }
        }
    }

    if (nearestTree) {
        nearestTree->reserved = true;
        unit.treeTileTargetIdx = tile;
        unit.treeTargetIdx = nearestTree->idx;
        return renderedTiles[tile].position + nearestTree->tileOffset;
    }
    return unit.pos;
}

void Map::removeTreeByIndex(int _treeTileTargetIdx, int _treeTargetIdx) {
    if (_treeTileTargetIdx == -1 || _treeTargetIdx == -1) 
        return;
    auto& tile = renderedTiles[_treeTileTargetIdx].occupyingEntities;
    tile.erase(std::remove(tile.begin(), tile.end(), _treeTargetIdx), tile.end());
}

void Map::drawBuildings() {
    for (auto& building : this->buildings) {
        building->draw();
    }
}

void Map::renderMap(int _screenWidth, int _screenHeight, int _tileSize) {
    // make this const reference whenever other thing runs im working on
    for (auto const& tile : renderedTiles) {
        if (!tile.hasBeenScouted) {
            DrawRectangle(tile.position.x, tile.position.y, _tileSize, _tileSize, GRAY);
        }
        else {
            DrawRectangle(tile.position.x, tile.position.y, _tileSize, _tileSize, tile.tileColor);
            for (auto& entity : tile.occupyingEntities) {
                DrawRectangle(tile.position.x + entity.tileOffset.x,
                    tile.position.y + entity.tileOffset.y,
                    _tileSize / 8, _tileSize / 8, { entity.entityColor });
            }
        }
    }
}