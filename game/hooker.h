//
// Created by GiaKhanhVN on 4/7/2025.
//

#ifndef TETISENGINE_HOOKER_H
#define TETISENGINE_HOOKER_H
#include <iostream>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>

using namespace std;

class ExecutionContext {
protected:
    unordered_map<int, function<void()>> ALL_EXECUTION_SCHEDULED;
    mutex mtx; // lock the cleanup queue
    queue<int> unhookQueue;

    int scheduledTasks = 0;
    bool stopped = false;
public:
    void unhook(int execId);
    int hook(function<void()> function);

    bool isRunning();
    void stop();

    void execute();
};

#endif //TETISENGINE_HOOKER_H
