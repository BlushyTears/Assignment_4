#pragma once

#include "raylib.h"
#include "resource_dir.h"	

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <algorithm>
#include <string>
#include <queue>
#include <chrono>

#include "MapManager.h"

struct Node {
	int x;
	int y;
};

struct Connection {
	int cost{ INT_MAX };
	Node fromNode;
	Node toNode;

	int getCost() { return cost; }
	Node getFromNode() { return fromNode; }
	Node getToNode() { return toNode; }
};

struct NodeRecord {
	Node node;
	Connection connection;
	int costSoFar{ INT_MAX };
	int estimatedTotalCost;

	int getCostSoFar() {
		return costSoFar;
	}
};

inline bool operator==(const Node& left, const Node& right) {
	return left.x == right.x && left.y == right.y;
}

inline bool operator!=(const Node& left, const Node& right) {
    return !(left.x == right.x && left.y == right.y);
}

inline bool operator==(const NodeRecord& left, const Node& right) {
	return left.node == right;
}

inline bool operator==(const Node& left, const NodeRecord& right) {
	return left == right.node;
}

inline bool operator==(const NodeRecord& left, const NodeRecord& right) {
	return left.node == right.node;
}

struct Graph {
	std::vector<NodeRecord> chartedGraph;

	Graph(std::vector<Vector2>& paths) {
		for (int i = 0; i < paths.size(); i++) {
			NodeRecord temp;
			temp.node.x = paths[i].x;
			temp.node.y = paths[i].y;
			temp.costSoFar = INT_MAX;
			chartedGraph.push_back(temp);
		}
	}

    void addLatestNodeToGraph(std::vector<Vector2>& paths) {
        if (paths.empty() || chartedGraph.size() == paths.size()) {
            return;
        }

        NodeRecord temp;
        temp.node.x = paths.back().x;
        temp.node.y = paths.back().y;
        temp.costSoFar = INT_MAX;
        chartedGraph.push_back(temp);
    }

	std::vector<Connection> getConnections(const Node& n, const std::vector<std::vector<int>>& neighboors) {
		std::vector<Connection> out;
        if (neighboors.size() == 0)
            return out;
		for (int i = 0; i < chartedGraph.size(); i++) {
			if (chartedGraph[i].node == n) {
				for (int idx : neighboors[i]) {
					Connection c;
					c.cost = 1;
					c.fromNode = n;
					c.toNode = chartedGraph[idx].node;
					out.push_back(c);
				}
			}
		}
		return out;
	}
};

struct ChartedMap {
    Graph* graph = nullptr;
    std::vector<Vector2> walkablePaths;
    std::vector<std::vector<int>> walkablePathsNeighboors;
    std::vector<NodeRecord> openList;

    int tileSize = 10;
    float xAccumulator;
    float yAccumulator;

    ChartedMap(int _tileSize) {
        tileSize = _tileSize;
        computeNeighboors();
        graph = new Graph(walkablePaths);
        openList = std::vector<NodeRecord>(10000);
    }

    // This map sets up everything related to everything non-algorithmic-related
    ChartedMap(std::string mapData, int _tileSize) {
        xAccumulator = 0;
        tileSize = _tileSize;
        yAccumulator = tileSize;

        for (int i = 0; i < mapData.length(); i++) {
            xAccumulator += tileSize;
            if (mapData[i] == 'G') {
                walkablePaths.push_back({ xAccumulator , yAccumulator });
            }
            if (mapData[i] == 'M') {
                walkablePaths.push_back({ xAccumulator , yAccumulator });
            }
            if (mapData[i] == '\n') {
                yAccumulator += tileSize;
                xAccumulator = 0;
            }
        }
        computeNeighboors();
        graph = new Graph(walkablePaths);
        openList = std::vector<NodeRecord>(graph->chartedGraph.size());
    }

    NodeRecord getSmallestNodeByCost(std::vector<NodeRecord>& list) {
        int bestIndex = 0;
        for (int i = 1; i < list.size(); i++) {
            if (list[i].costSoFar < list[bestIndex].costSoFar) {
                bestIndex = i;
            }
        }
        return list[bestIndex];
    }

    NodeRecord smallestEstimate(std::vector<NodeRecord>& list) {
        int bestIndex = 0;
        for (int i = bestIndex; i < list.size(); i++) {
            if (list[i].estimatedTotalCost < list[bestIndex].estimatedTotalCost) {
                bestIndex = i;
            }
        }
        return list[bestIndex];
    }

    float euclidianDistance(NodeRecord& nodeRecord, Vector2& start, Vector2& goal) {
        return sqrtf(pow((goal.x - nodeRecord.node.x), 2) + pow((goal.y - nodeRecord.node.y), 2));
    }

    std::vector<Connection> AStar(Vector2& start, Vector2& goal, std::vector<std::vector<int>>& neighbooringNodes) {
        NodeRecord startRecord;
        startRecord.node = Node{ (int)start.x, (int)start.y };
        startRecord.costSoFar = 0;
        startRecord.connection = Connection();
        startRecord.estimatedTotalCost = euclidianDistance(startRecord, start, goal);

        openList.clear();
        openList.push_back(startRecord);
        std::vector<NodeRecord> closedList;

        NodeRecord current;

        while (openList.size() > 0) {
            current = smallestEstimate(openList);

            if (current.node.x == goal.x && current.node.y == goal.y) {
                break;
            }
            auto tempIt = std::find(openList.begin(), openList.end(), current);
            openList.erase(tempIt);
            closedList.push_back(current);

            std::vector<Connection> connections = graph->getConnections(current.node, neighbooringNodes);
            int endNodeHeuristic = 1;

            for (auto& connection : connections) {
                Node endNode = connection.getToNode();

                int endNodeCost = current.getCostSoFar() + connection.getCost();
                NodeRecord endNodeRecord = NodeRecord();

                auto closedIt = std::find(closedList.begin(), closedList.end(), endNode);
                auto openIt = std::find(openList.begin(), openList.end(), endNode);

                if (closedIt != closedList.end()) {
                    endNodeRecord = *closedIt;

                    if (endNodeRecord.costSoFar <= endNodeCost)
                        continue;

                    closedList.erase(closedIt);
                    endNodeHeuristic = endNodeRecord.estimatedTotalCost - endNodeRecord.costSoFar;
                }
                else if (openIt != openList.end()) {
                    endNodeRecord = *openIt;
                    if (endNodeRecord.costSoFar <= endNodeCost) {
                        continue;
                    }

                    endNodeHeuristic = endNodeRecord.connection.cost - endNodeRecord.costSoFar;
                }
                else {
                    endNodeRecord.node = endNode;
                    endNodeHeuristic = euclidianDistance(endNodeRecord, start, goal);
                }

                endNodeRecord.costSoFar = endNodeCost;
                endNodeRecord.connection = connection;
                endNodeRecord.estimatedTotalCost = endNodeCost + endNodeHeuristic;

                if (openIt == openList.end()) {
                    openList.push_back(endNodeRecord);
                }
            }
        }

        std::vector<Connection> path;
        if (current.node.x == goal.x && current.node.y == goal.y) {
            while (current.node != startRecord.node) {
                path.push_back(current.connection);

                Node parrentNode = current.connection.getFromNode();
                auto it = std::find(closedList.begin(), closedList.end(), parrentNode);

                if (it == closedList.end()) {
                    break;
                }
                current = *it;
            }
            reverse(path.begin(), path.end());
        }
        return path;
    }

    // as we unlock new tiles we probably wanna put them in the list
    // only needs to loop through: walkablePaths.size() - 1;
    void computeNewNeighboors(int i) {
        // Resize to match walkablePaths size if needed
        if (walkablePathsNeighboors.size() < walkablePaths.size()) {
            walkablePathsNeighboors.resize(walkablePaths.size());
        }

        for (int j = 0; j < walkablePaths.size(); j++) {
            if (i == j) continue;

            int dx = walkablePaths[j].x - walkablePaths[i].x;
            int dy = walkablePaths[j].y - walkablePaths[i].y;

            bool isHorizontal = (abs(dx) == tileSize && dy == 0);
            bool isVertical = (abs(dy) == tileSize && dx == 0);
            bool isDiagonal = (abs(dx) == tileSize && abs(dy) == tileSize);

            if (isHorizontal || isVertical) {
                walkablePathsNeighboors[i].push_back(j);
                walkablePathsNeighboors[j].push_back(i);
                continue;
            }

            if (isDiagonal) {
                int bx = walkablePaths[i].x + dx;
                int by = walkablePaths[i].y;

                int cx = walkablePaths[i].x;
                int cy = walkablePaths[i].y + dy;

                bool horizontalExists = false;
                bool verticalExists = false;

                for (int k = 0; k < walkablePaths.size(); k++) {
                    if (walkablePaths[k].x == bx && walkablePaths[k].y == by)
                        horizontalExists = true;
                    if (walkablePaths[k].x == cx && walkablePaths[k].y == cy)
                        verticalExists = true;
                }

                if (horizontalExists && verticalExists) {
                    walkablePathsNeighboors[i].push_back(j);
                    walkablePathsNeighboors[j].push_back(i);
                }
            }
        }
    }

    void computeNeighboors() {
        walkablePathsNeighboors.clear();
        walkablePathsNeighboors.resize(walkablePaths.size() * 4);

        for (int i = 0; i < walkablePaths.size(); i++) {
            computeNewNeighboors(i);
        }
    }
};