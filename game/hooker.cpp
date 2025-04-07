//
// Created by GiaKhanhVN on 4/7/2025.
//

#include "hooker.h"

int ExecutionContext::hook(function<void()> function) {
    std::lock_guard<std::mutex> lock(mtx);
    int execId = scheduledTasks++;
    ALL_EXECUTION_SCHEDULED[execId] = function;
    return execId;
}

void ExecutionContext::unhook(int execId) {
    lock_guard<std::mutex> lock(mtx);
    unhookQueue.push(execId);
}

void ExecutionContext::execute() {
    queue<int> localUnhookQueue;
    {
        lock_guard<mutex> lock(mtx);
        swap(unhookQueue, localUnhookQueue);
    }

    while (!localUnhookQueue.empty()) {
        int execId = localUnhookQueue.front();
        localUnhookQueue.pop();
        ALL_EXECUTION_SCHEDULED.erase(execId);
        cout << "Unhooked task with execId: " << execId << endl;
    }

    for (auto&[taskId, task]: ALL_EXECUTION_SCHEDULED) {
        task();
    }
}

bool ExecutionContext::isRunning() {
    return !this->stopped;
}

void ExecutionContext::stop() {
    std::lock_guard<std::mutex> lock(mtx);
    this->stopped = true;
}