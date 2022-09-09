#pragma once

#include <list>
#include <functional>
#include <ucontext.h>
#include <map>
#include <fcntl.h>

class Fiber;

class XFiber {
public:
    XFiber();
    ~XFiber();
    ucontext_t * getCtx() { return &ctx_;}
    void CreateFiber(std::function<void()> run);
    void Dispatch();
    void Yield();
    void RegisterFdToScheduler(int fd, bool is_write);
    void UnregisterFdFromScheduler(int fd);
    void SwitchToScheduler();
    static XFiber * GetInstance () {
        static thread_local XFiber xFiber;
        return & xFiber;
    }
private:
    struct IoWaitingFiber{
        Fiber * w_;
        Fiber * r_;
    };

    int epfd_;
    std::map<int, IoWaitingFiber> io_waiting_fibers_;
    std::list<Fiber * > ready_fibers_;    //
    std::list<Fiber * > running_fibers_;  //
    Fiber * cur_fiber_;
    ucontext_t ctx_;
};

class Fiber {
public:
    Fiber(std::function<void()> run, XFiber * xfiber, int stack_size = 4096);
    ~Fiber();
    ucontext_t * getCtx() { return &ctx_;}
    bool IsFinished() const {
      return status_ == Finished;
    }
    static void Start(Fiber * fiber);  //入口函数
private:
    using Status = enum {
      Running,
      Finished
    };
    std::function<void()> run_;
    ucontext_t ctx_;
    Status status_;
    ssize_t stack_size_;
    char * stack_;
};