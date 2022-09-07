#include "xsocket.h"
#include <iostream>

Listener::Listener() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if(fd_ == -1) {
        perror("create listen fd");
        exit(0);
    }
    int option = ::fcntl(fd_, F_GETFL);
    ::fcntl(fd_, F_SETFL, option | O_NONBLOCK);
}

void Listener::Listen(int port) {
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
    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    socklen_t len = sizeof client_addr;
    int connfd = ::accept(fd_, (sockaddr*)&client_addr, &len);
    // std::cout << connfd << std::endl;
    if(connfd == -1) {
        if(errno != EAGAIN) {
            perror("accept");
            exit(0);
        }
    } else {
        int option = ::fcntl(connfd, F_GETFL);
        ::fcntl(connfd, F_SETFL, option | O_NONBLOCK);
    }
    return connfd;
}