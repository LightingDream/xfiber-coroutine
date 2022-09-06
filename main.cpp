#include "xfiber.h"
#include <iostream>
int main(int, char**) {
    XFiber xfiber;
    xfiber.CreateFiber([&]{
        std::cout << "I am coroutine A" << std::endl;
        std::cout << "I am coroutine A" << std::endl;
        xfiber.Yield();
        std::cout << "I am coroutine A" << std::endl;
        std::cout << "I am coroutine A" << std::endl;
    });
    xfiber.CreateFiber([&]{
        std::cout << "I am coroutine B" << std::endl;
        std::cout << "I am coroutine B" << std::endl;
        xfiber.Yield();
        std::cout << "I am coroutine B" << std::endl;
        std::cout << "I am coroutine B" << std::endl;
    });
    xfiber.Dispatch();
    return 0;
}
