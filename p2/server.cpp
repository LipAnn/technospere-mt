#include "server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <set>
#include <cstdio>
#include <map>
#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>

const int MAX_EVENTS = SOMAXCONN;

using std::cin;
using std::cout;
using std::endl;
using std::set;
using std::fflush;
using std::map;
using std::min;

Server::Server(int port) : max_buf_size_(1024), message_buf_(), clients_() {
    sockaddr_in socket_addr;
    bzero(&socket_addr, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    set_nonblock(listen_socket_);
    int optval = 1;
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bind(listen_socket_, (sockaddr*)(&socket_addr), sizeof(socket_addr));
}

Server::~Server() {
    shutdown(listen_socket_, SHUT_RDWR);
}

void Server::terminateConnection(int slave_socket, int epoll) {
    clients_.erase(slave_socket);
    message_buf_.erase(slave_socket);
    epoll_ctl(epoll, EPOLL_CTL_DEL, slave_socket, nullptr);
    shutdown(slave_socket, SHUT_RDWR);
    cout << "connection terminated" << "\n";
    cout.flush();
}

int Server::recvMessage(int slave_socket, int epoll) {
    char buf_recv[max_buf_size_];
    int recv_bytes = recv(slave_socket, buf_recv, max_buf_size_, 0);
    if (recv_bytes <= 0) {
        return -1;
    }
    message_buf_[slave_socket] += string(buf_recv, buf_recv + recv_bytes);
    if (message_buf_[slave_socket].back() == '\n') {
        return 1;
    }
    return 0;
}

void Server::sendMessage(int slave_socket) {
    for (auto it = message_buf_[slave_socket].begin(); it < message_buf_[slave_socket].end(); it += max_buf_size_) {
        string::iterator it2 = min(it + max_buf_size_, message_buf_[slave_socket].end());
        for (auto client : clients_) {
            send(client, string(it, it2).c_str(), it2 - it, MSG_NOSIGNAL);
        }
    }
    cout << message_buf_[slave_socket];
    cout.flush();
    message_buf_[slave_socket] = "";
}

void Server::run() {
    int epoll = epoll_create1(0);
    epoll_event event;
    event.data.fd = listen_socket_;
    event.events = EPOLLIN;
    epoll_ctl(epoll, EPOLL_CTL_ADD, listen_socket_, &event);

    epoll_event *events = new epoll_event[MAX_EVENTS];

    listen(listen_socket_, SOMAXCONN);
    while (true) {
        int N = epoll_wait(epoll, events, MAX_EVENTS, -1);
        for (int i = 0; i < N; ++i) {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                terminateConnection(events[i].data.fd, epoll);
                continue;
            }
            if (events[i].data.fd == listen_socket_) {
                int slave_socket = accept(listen_socket_, 0, 0);
                clients_.insert(slave_socket);
                set_nonblock(slave_socket);
                cout << "accepted connection " <<  endl;
                cout.flush();
                message_buf_[slave_socket] = "";
                const char *s = "Welcome\n";
                send(slave_socket, s, strlen(s), MSG_NOSIGNAL);
                epoll_event slave_event;
                slave_event.data.fd = slave_socket;
                slave_event.events = EPOLLIN;
                epoll_ctl(epoll, EPOLL_CTL_ADD, slave_socket, &slave_event);
            } else {
                int slave_socket = events[i].data.fd;
                int flag = recvMessage(slave_socket, epoll);
                if (flag == -1) {
                    terminateConnection(slave_socket, epoll);
                    continue;
                }
                if (flag) {
                    sendMessage(slave_socket);
                }
            }        
        }
    }
}


int main() {
    //std::ios_base::sync_with_stdio(false);
    //cin.tie(nullptr);
    //set_nonblock(0);
    Server srv(3100);
    srv.run();
    return 0;
}


