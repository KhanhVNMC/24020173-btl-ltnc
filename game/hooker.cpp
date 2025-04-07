//
// Created by GiaKhanhVN on 4/7/2025.
//

#include "hooker.h"

int ExecutionContext::hook(function<void()> function) {
    lock_guard<mutex> lock(mtx);
    int execId = scheduledTasks++;
    ALL_EXECUTION_SCHEDULED[execId] = function;
    return execId;
}

void ExecutionContext::unhook(int execId) {
    lock_guard<std::mutex> lock(mtx);
    unhookQueue.push(execId);
}

void ExecutionContext::execute() {
    // fucking black magic
    queue<int> localUnhookQueue;
    {
        lock_guard<mutex> lock(mtx);
        swap(unhookQueue, localUnhookQueue);
    }

    // unhook every tasks in the queue
    while (!localUnhookQueue.empty()) {
        int execId = localUnhookQueue.front();
        localUnhookQueue.pop();
        ALL_EXECUTION_SCHEDULED.erase(execId);
        cout << "Unhooked task with execId: " << execId << endl;
    }

    // execute all tasks left in the queue (could be 0)
    for (auto&[taskId, task]: ALL_EXECUTION_SCHEDULED) {
        task();
    }
}

bool ExecutionContext::isRunning() {
    return !this->stopped;
}

void ExecutionContext::stop() {
    lock_guard<mutex> lock(mtx);
    this->stopped = true;
}

void ExecutionContext::pushEvent(SDL_Event event) {
    lock_guard<std::mutex> lock(mtx);
    eventQueue.push(event);
}

bool ExecutionContext::popEvent(SDL_Event& event) {
    lock_guard<std::mutex> lock(mtx);
    if (eventQueue.empty()) {
        return false;
    }
    event = eventQueue.front();
    eventQueue.pop();
    return true;
}