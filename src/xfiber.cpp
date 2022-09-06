#include "xfiber.h"
#include <iostream>

XFiber::XFiber() {
    ready_fibers_.clear();
    running_fibers_.clear();
    cur_fiber_ = nullptr;
    getcontext(&ctx_);
}

XFiber::~XFiber() {

}

void XFiber::CreateFiber(std::function<void()> run) {
  ready_fibers_.push_back(new Fiber(run, this));
}

void XFiber::Dispatch() {
  while(true){
    if(ready_fibers_.size() == 0) continue;
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
  }
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
    free(ctx_.uc_stack.ss_sp);
}

Fiber::Fiber(std::function<void()> run, XFiber * xfiber) {
    run_ = run;
    getcontext(&ctx_);
    ctx_.uc_stack.ss_sp = new char[4096];
    ctx_.uc_stack.ss_size = 4096;
    ctx_.uc_link = xfiber->getCtx();
    status_ = Running;
    makecontext(&ctx_, (void(*)())(Fiber::Start), 1, this);
}
