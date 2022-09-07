#include "xfiber.h"
#include "xsocket.h"

#include <string.h>
#include <strings.h>
#include <iostream>

int main(int, char**) {
    XFiber xfiber;
    // xfiber.CreateFiber([&]{
    //     std::cout << "I am coroutine A" << std::endl;
    //     std::cout << "I am coroutine A" << std::endl;
    //     xfiber.Yield();
    //     std::cout << "I am coroutine A" << std::endl;
    //     std::cout << "I am coroutine A" << std::endl;
    // });
    // xfiber.CreateFiber([&]{
    //     std::cout << "I am coroutine B" << std::endl;
    //     std::cout << "I am coroutine B" << std::endl;
    //     xfiber.Yield();
    //     std::cout << "I am coroutine B" << std::endl;
    //     std::cout << "I am coroutine B" << std::endl;
    // });
    xfiber.CreateFiber([&]{
        Listener listener;
        listener.Listen(8888);
        while(true) {
            // sleep(2);
            int connfd = listener.Accept();
            if(connfd <= 0) continue;
            xfiber.CreateFiber([&]{
                char buf[1024] = {0};
                while(true) {
                    sleep(2);
                    int len = ::read(connfd, buf, 1024);
                    std::cout << "read : " << buf << std::endl;
                    ::write(connfd, buf, strlen(buf));
                    ::bzero(buf, sizeof buf);
                }
            });
            xfiber.Yield();
        }
    });
    xfiber.Dispatch();
    return 0;
}
