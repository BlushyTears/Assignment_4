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
    bool timerFinished = false;
    float timeCap = 1;

    void setNewTimer(int _newCap) {
        timerFinished = false;
        timeCap = _newCap;
        currTimePassed = 0;
    }

    void updateTimer() {
        if (!timerFinished) {
            currTimePassed += GetFrameTime();

            if (currTimePassed >= timeCap) {
                timerFinished = true;
            }
        }
    }

    bool hasTimerEnded() {
        return timerFinished;
    }
        
    void reset() {
        currTimePassed = 0;
        timerFinished = false;
    }
};


