#include "xsocket.h"
#include <iostream>

Listener::Listener() {
    
}

void Listener::Listen(uint16_t port) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if(fd_ == -1) {
        perror("create listen fd");
        exit(0);
    }
    int option = ::fcntl(fd_, F_GETFL);
    ::fcntl(fd_, F_SETFL, option | O_NONBLOCK);

    sockaddr_in addr_;
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    addr_.sin_addr.s_addr = INADDR_ANY;
    socklen_t optval = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
    
    int ret = ::bind(fd_, (sockaddr*)&addr_, sizeof addr_);
    if(ret == -1) {
        perror("bind");
        exit(0);
    }

    ret = ::listen(fd_, 5);
    if(ret == -1) {
        perror("listen");
        exit(0);
    }
}

int Listener::Accept() {
    XFiber * xfiber = XFiber::GetInstance();
    while(true) {
        sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        socklen_t len = sizeof client_addr;
        int connfd = ::accept(fd_, (sockaddr*)&client_addr, &len);
        // std::cout << connfd << std::endl;
        if(connfd > 0) {
            int option = ::fcntl(connfd, F_GETFL);
            ::fcntl(connfd, F_SETFL, option | O_NONBLOCK);
            return connfd;
        }
        if( errno == EAGAIN ) {
            xfiber->RegisterFdToScheduler(fd_, false);
            xfiber->SwitchToScheduler();
            // yield;
        } else if(errno == EINTR) {
            continue;
        } else {
            perror("accept");
            exit(0);
        }
    }
    return -1;
}

Connection::Connection(int fd) {
    fd_ = fd;
}

int Connection::Read(char* buf, size_t sz) {
    XFiber * xfiber = XFiber::GetInstance();
    while(true) {
        int len = read(fd_, buf, sz);
        std::cout << len << std::endl;
        if(len >= 0) {
            return len;
        }
        if(errno != EAGAIN && errno != EINTR) {
            perror("read");
            return -1;
        }
        if(errno == EAGAIN) {
            std::cout << "read fd[" << fd_ << "] yield" << std::endl;
            xfiber->RegisterFdToScheduler(fd_, false);
            xfiber->SwitchToScheduler();
        }
    }
    return -1;
}

int Connection::Write(const char* buf, size_t sz) {
    XFiber * xfiber = XFiber::GetInstance();
    int write_bytes = 0;
    while(write_bytes < sz) {
        int len = write(fd_, buf + write_bytes, sz - write_bytes);
        if(len > 0) {
            write_bytes += len;
        }
        else if(errno != EAGAIN && errno != EINTR) {
            perror("write");
            return -1;
        }
        else if(errno == EAGAIN) {
            std::cout << "write fd[" << fd_ << "] yield" << std::endl;
            xfiber->RegisterFdToScheduler(fd_, true);
            xfiber->SwitchToScheduler();
        }
    }
    return sz;
}

void Connection::Close() {
    if(fd_ < 0) {
        return;
    }
    XFiber * xfiber = XFiber::GetInstance();
    xfiber->UnregisterFdFromScheduler(fd_);
    close(fd_);
}