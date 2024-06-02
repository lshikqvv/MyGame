#pragma once

#include <arpa/inet.h>   // inet_addr
#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind, listen, accept
#include <unistd.h>      // close
#include <string.h>      // memset

#include <iostream>
#include <string>

namespace connection
{
    class Socket
    {
    public:
        int sockfd;
        int cnctfd;
        char buff[1024] = {0};

        int create(const char* ip_addr, int port);
        int serve(int port);
        void request(std::string str);
        std::string response();
        int finish();
    };
}
