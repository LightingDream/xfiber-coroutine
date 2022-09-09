#pragma once

#include <arpa/inet.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "xfiber.h"
class Listener {
public:
    Listener();
    void Listen(uint16_t port);
    int Accept();
private:
    int fd_;
};

class Connection {
public:
    Connection(int fd);
    int Read(char* buf, size_t sz);
    int Write(const char * buf, size_t sz);
    void Close();
private:
    int fd_;
};
