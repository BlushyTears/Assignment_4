#pragma once

#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <memory>

struct UnitBase;
struct Resource;
struct ResourceTracker;

// Idea: This struct should ascribe tasks to units using FSM

struct TaskManager {
	ResourceTracker* resourceTracker = nullptr;
	std::vector<std::unique_ptr<UnitBase>>* units = nullptr;

	TaskManager(ResourceTracker* _rt, std::vector<std::unique_ptr<UnitBase>>* _u) {
		resourceTracker = _rt;
		units = _u;
	}

	void delegateTasks() {
	
	}
};
