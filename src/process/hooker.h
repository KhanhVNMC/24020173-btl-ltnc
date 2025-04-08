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
#include <SDL_events.h>

using namespace std;

class ExecutionContext {
protected:
    unordered_map<int, function<void()>> ALL_EXECUTION_SCHEDULED;
    mutex mtx; // lock the cleanup queue AND SDL event(s)

    // queue for unhooking tasks
    queue<int> unhookQueue;

    // queue for SDL thingy
    queue<SDL_Event> eventQueue;

    // the amount of tasks this context received (ever)
    int scheduledTasks = 0;
    bool stopped = false;

    // function to run
    unordered_map<int, function<void()>> ON_UNHOOK_SUCCESS_CALLBACK;
public:
    function<void()> contextReturnMainMenu = nullptr;

    /**
     * Hook a task into this Context
     * @param function the task
     * @param type (optional) what the task for [default = Unknown]
     * @return the task ID
     */
    int hook(function<void()> function, string type = "Unknown");

    /**
     * Unhook a task from this Context
     * @param execId the task ID (given by hook())
     * @param onUnhookSuccess the function to run on unhook success (optional)
     */
    void unhook(int execId, function<void()> onUnhookSuccess = nullptr);

    /**
     * @return true if the context is still alive
     */
    bool isRunning();

    /**
     * Gracefully kill the context
     */
    void stop();

    /**
     * Execute this context (thread safe)
     */
    void execute();

    /**
     * Push an SDL Event into the intercom queue (thread safe)
     * @param event
     */
    void pushEvent(SDL_Event event);

    /**
     * Pop an event out of the intercom queue
     * @param event the same as SDL_PollEvent()
     * @return true if exists
     */
    bool popEvent(SDL_Event& event);
};

#endif //TETISENGINE_HOOKER_H
