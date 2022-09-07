#pragma once

#include <arpa/inet.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

class Listener {
public:
    Listener();
    void Listen(int port);
    int Accept();
private:
    int fd_;
};

