#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <queue>
#include "raylib.h"

using namespace std;
using std::string;

// It seems like all units and buildings use their own timer rather than a queue timing one thing at a time
// So we'll need a pool of timers associated with some form of result once the timer ends.
struct Timer {
    float currTimePassed = 0;
    bool timerFinished = true;

    void setNewTimer(float _newCap) {
        timerFinished = false;
        currTimePassed = _newCap;
    }

    void updateTimer() {
        if (!timerFinished) {
            currTimePassed -= GetFrameTime();

            if (currTimePassed <= 0) {
                timerFinished = true;
            }
        }
    }

    bool hasTimerEnded() {
        return timerFinished;
    }
};