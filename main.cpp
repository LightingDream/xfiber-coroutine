#include "xfiber.h"
#include "xsocket.h"

#include <string.h>
#include <strings.h>
#include <iostream>

int main(int, char**) {
    XFiber * xfiber = XFiber::GetInstance();
    xfiber->CreateFiber([&]{
        Listener listener;
        listener.Listen(8888);
        while(true) {
            int connfd = listener.Accept();
            if(connfd < 0) continue;
            Connection conn(connfd);
            xfiber->CreateFiber([&]{
                char buf[1024] = {0};
                // while(true) {
                    int res = conn.Read(buf, 1024);
                    if(res <= 0) {
                        conn.Close();
                        return;
                    }
                    std::cout << "recv : " << buf << std::endl;
                    if(conn.Write(buf, strlen(buf)) <= 0) {
                        conn.Close();
                        return;
                    }
                    ::bzero(buf, sizeof buf);
                    
                // }
            });
        }
    });
    xfiber->Dispatch();
    return 0;
}
