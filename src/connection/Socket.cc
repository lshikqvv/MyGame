#include "Socket.h"

using namespace std;
using namespace connection;

int Socket::create(const char* ip_addr, int port)
{
    struct sockaddr_in addr;

    /*   socket(domain, type, protocol)
    **     domain: AF_INET(IPv4), AF_INET6(IPv6)
    **     type: SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
    **     protocol: 0
    */
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));  // Fill with 0
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);    // Host byte order to Network byte order

    this->cnctfd = connect(this->sockfd, (struct sockaddr *)&addr, sizeof(addr));

    if(inet_pton(AF_INET, ip_addr, &addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (this->cnctfd < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    return this->sockfd;
}

int Socket::serve(int port)
{
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    int opt = 1;

    this->cnctfd = socket(AF_INET, SOCK_STREAM, 0);

    if (this->cnctfd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(this->cnctfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));  // Fill with 0
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);    // Host byte order to Network byte order

    if (bind(this->cnctfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(this->cnctfd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    sockfd = accept(this->cnctfd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);

    if (sockfd < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    return this->sockfd;
}

void Socket::request(string str)
{
    send(this->sockfd, str.c_str(), str.size(), 0);

    cout << "Sent: " << str << endl;
}

string Socket::response()
{
    char buff[1024] = {0};
    read(this->sockfd, this->buff, 1024);
    string str = string(this->buff);

    cout << "Read: " << buff << endl;
    return str;
}

int Socket::finish()
{
    close(this->sockfd);
    shutdown(this->sockfd, SHUT_RDWR);

    return 0;
}
