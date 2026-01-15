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
	// Below is just for aStar
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
    std::vector<Vector2> scoutedPaths; // is it possible to walk here?
    std::vector<Vector2> walkablePaths; // is it possible to walk here?
    std::vector<std::vector<int>> walkablePathsNeighboors;
    std::vector<std::vector<int>> ScoutedPathsNeighboors;

    int tileSize;
    float xAccumulator;
    float yAccumulator;

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

        computeNeighboors(walkablePaths, walkablePathsNeighboors);
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

    std::vector<Connection> AStar(Vector2& start, Vector2& goal, std::vector<Vector2>& pathType, std::vector<std::vector<int>>& neighbooringNodes) {
        Graph graph(pathType);

        NodeRecord startRecord;
        startRecord.node = Node{ (int)start.x, (int)start.y };
        startRecord.costSoFar = 0;
        startRecord.connection = Connection();
        startRecord.estimatedTotalCost = euclidianDistance(startRecord, start, goal);

        std::vector<NodeRecord> openList;
        openList.push_back(startRecord);
        std::vector<NodeRecord> closedList;

        NodeRecord current;

        while (openList.size() > 0) {
            current = smallestEstimate(openList);

            if (current.node.x == goal.x && current.node.y == goal.y) {
                break;
            }
            std::vector<Connection> connections = graph.getConnections(current.node, neighbooringNodes);
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
            auto tempIt = std::find(openList.begin(), openList.end(), current);
            openList.erase(tempIt);
            closedList.push_back(current);
        }

        std::vector<Connection> path;
        if (current.node.x == goal.x && current.node.y == goal.y) {
            while (current.node != startRecord.node) {
                path.push_back(current.connection);

                Node parrentNode = current.connection.getFromNode();
                auto it = std::find(closedList.begin(), closedList.end(), parrentNode);

                if (it == closedList.end()) {
                    if (parrentNode == startRecord.node) {
                        break;
                    }
                }
                current = *it;
            }
            reverse(path.begin(), path.end());
        }
        return path;
    }

    std::vector<Connection> dijkstra(Vector2& start, Vector2& goal, std::vector<Vector2>& pathType, std::vector<std::vector<int>>& neighbooringNodes) {
        Graph graph(pathType);

        NodeRecord startRecord;
        startRecord.node = Node{ (int)start.x, (int)start.y };
        startRecord.costSoFar = 0;
        startRecord.connection = Connection();

        std::vector<NodeRecord> openList;
        openList.push_back(startRecord);
        std::vector<NodeRecord> closedList;

        NodeRecord current;

        while (openList.size() > 0) {
            current = getSmallestNodeByCost(openList);

            if (current.node.x == goal.x && current.node.y == goal.y) {
                break;
            }

            std::vector<Connection> connections = graph.getConnections(current.node, neighbooringNodes);

            for (auto& connection : connections) {
                Node endNode = connection.getToNode();

                int endNodeCost = current.getCostSoFar() + connection.getCost();

                NodeRecord endNodeRecord = NodeRecord();
                if (std::find(closedList.begin(), closedList.end(), endNode) != closedList.end()) {
                    continue;
                }

                auto it = std::find(openList.begin(), openList.end(), endNode);

                // Is the endnodeRecord in open list
                if (it != openList.end()) {
                    endNodeRecord = *it;
                    if (endNodeRecord.costSoFar <= endNodeCost) {
                        continue;
                    }
                }
                else {
                    endNodeRecord.node = endNode;
                }
                endNodeRecord.costSoFar = endNodeCost;
                endNodeRecord.connection = connection;

                // is end node not in open list
                if (std::find(openList.begin(), openList.end(), endNode) == openList.end()) {
                    openList.push_back(endNodeRecord);
                }
            }
            auto temp = std::find(openList.begin(), openList.end(), current);
            openList.erase(temp);
            closedList.push_back(current);
        }

        std::vector<Connection> path;
        if (current.node.x == goal.x && current.node.y == goal.y) {
            while (current.node != startRecord.node) {
                path.push_back(current.connection);

                Node parrentNode = current.connection.getFromNode();
                auto it = std::find(closedList.begin(), closedList.end(), parrentNode);

                if (it == closedList.end()) {
                    if (parrentNode == startRecord.node) {
                        break;
                    }
                }
                current = *it;
            }
            reverse(path.begin(), path.end());
        }
        return path;
    }

    void computeNeighboors(std::vector<Vector2>& pathType, std::vector<std::vector<int>>& neighbooringNodes) {
        neighbooringNodes.clear();
        neighbooringNodes.resize(pathType.size() * 4);



        for (int i = 0; i < pathType.size(); i++) {
            for (int j = 0; j < pathType.size(); j++) {
                if (i == j) continue;

                int dx = pathType[j].x - pathType[i].x;
                int dy = pathType[j].y - pathType[i].y;

                bool isHorizontal = (abs(dx) == tileSize && dy == 0);
                bool isVertical = (abs(dy) == tileSize && dx == 0);
                bool isDiagonal = (abs(dx) == tileSize && abs(dy) == tileSize);

                if (isHorizontal || isVertical) {
                    neighbooringNodes[i].push_back(j);
                    continue;
                }

                if (isDiagonal) {
                    int bx = pathType[i].x + dx;
                    int by = pathType[i].y;

                    int cx = pathType[i].x;
                    int cy = pathType[i].y + dy;

                    bool horizontalExists = false;
                    bool verticalExists = false;

                    for (int k = 0; k < pathType.size(); k++) {
                        if (pathType[k].x == bx && pathType[k].y == by)
                            horizontalExists = true;
                        if (pathType[k].x == cx && pathType[k].y == cy)
                            verticalExists = true;
                    }

                    if (horizontalExists && verticalExists) {
                        neighbooringNodes[i].push_back(j);
                    }
                }
            }
        }
    }
};