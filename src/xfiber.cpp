#include "xfiber.h"
#include <iostream>
#include <sys/epoll.h>

XFiber::XFiber() {
    cur_fiber_ = nullptr;
    epfd_ = epoll_create1(0);
    if(epfd_ < 0) {
        perror("epoll_create");
        exit(0);
    }
}

XFiber::~XFiber() {
    ready_fibers_.clear();
    running_fibers_.clear();
}

void XFiber::CreateFiber(std::function<void()> run) {
    ready_fibers_.push_back(new Fiber(run, this));
}

void XFiber::Dispatch() {
    while(true){
        // if(ready_fibers_.size() == 0) continue;
        running_fibers_ = std::move(ready_fibers_);
        ready_fibers_.clear();
        for(auto fiber : running_fibers_) {
            cur_fiber_ = fiber;
            // std::cout << "Before Sched" << std::endl;
            swapcontext(&ctx_, cur_fiber_->getCtx());
            // std::cout << "End Sched" << std::endl;
            if(cur_fiber_->IsFinished()) {
                delete cur_fiber_;
            }
            cur_fiber_ = nullptr;
        }
        running_fibers_.clear();

        epoll_event events[128];
        int ready = epoll_wait(epfd_, events, 128, 5);
        if(ready < 0) {
            perror("epoll_wait");
            exit(0);
        }
        
        for(int i = 0; i < ready; ++i) {
            auto & ev = events[i];
            int fd = ev.data.fd;
            auto iter = io_waiting_fibers_.find(fd);
            if(iter == io_waiting_fibers_.end()) {
                continue;
            }
            //唤醒
            if(ev.events | EPOLLIN && iter->second.r_ != NULL) {
                ready_fibers_.push_back(iter->second.r_);
            }
            if(ev.events | EPOLLOUT && iter->second.w_ != NULL) {
                ready_fibers_.push_back(iter->second.w_);
            }  
        }
    }
}

void XFiber::RegisterFdToScheduler(int fd, bool is_write) {
    auto iter = io_waiting_fibers_.find(fd);
    if(iter == io_waiting_fibers_.end()) {
        IoWaitingFiber wb;
        if(!is_write) {
            wb.r_ = cur_fiber_;
            wb.w_ = nullptr;
        } else {
            wb.r_ = nullptr;
            wb.w_ = cur_fiber_;
        }
        io_waiting_fibers_.insert(std::make_pair(fd, wb));

        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;

        if(::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
            perror("epoll_ctl add");
            exit(0);
        }
    } else {
        if(!is_write) {
            iter->second.r_ = cur_fiber_;
        } else {
            iter->second.w_ = cur_fiber_;
        }
    }
}

void XFiber::UnregisterFdFromScheduler(int fd) {
    auto iter = io_waiting_fibers_.find(fd);
    if(iter == io_waiting_fibers_.end()) {
        return;
    }
    io_waiting_fibers_.erase(iter);
    if(epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        perror("epoll_ctl del");
        exit(0);
    }
}

void XFiber::SwitchToScheduler() {
    swapcontext(cur_fiber_->getCtx(), &ctx_);
}

void XFiber::Yield() {
    ready_fibers_.push_back(cur_fiber_);
    swapcontext(cur_fiber_->getCtx(), &ctx_);
}

void Fiber::Start(Fiber * fiber) {
    fiber->run_();
    fiber->status_ = Finished;
}

Fiber::~Fiber(){
    delete stack_;
}

Fiber::Fiber(std::function<void()> run, XFiber * xfiber, int stack_size) {
    run_ = run;
    getcontext(&ctx_);
    stack_size_ = stack_size;
    stack_ = new char[stack_size];
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stack_size_;
    ctx_.uc_link = xfiber->getCtx();
    status_ = Running;
    makecontext(&ctx_, (void(*)())(Fiber::Start), 1, this);
}
