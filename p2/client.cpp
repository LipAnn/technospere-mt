#include "server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <set>
#include <arpa/inet.h>
#include <netinet/in.h>

using std::cin;
using std::cout;
using std::endl;
using std::set;
using std::getline;
using std::min;
using std::string;

Client::Client() : max_buf_size_(1024), recv_message_() {
    client_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    set_nonblock(client_socket_);
    int optval = 1;
    setsockopt(client_socket_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}


Client::~Client() {
    shutdown(client_socket_, SHUT_RDWR);
}

int Client::recvMessage() {
    char buf_recv[max_buf_size_];
    int recv_bytes = recv(client_socket_, buf_recv, max_buf_size_, 0); 
    if (recv_bytes <= 0) {
        return -1;
    }

    recv_message_ += string(buf_recv, buf_recv + recv_bytes);
    if (recv_message_.back() == '\n') {
        return 1;
    }
    return 0;
}

void Client::sendMessage() {
    string buf_send;
    getline(cin, buf_send);
    buf_send += "\n";
    for (auto it = buf_send.begin(); it < buf_send.end(); it += max_buf_size_) {
        string::iterator it2 = min(it + max_buf_size_, buf_send.end());
        size_t send_bytes = it2 - it;
        send(client_socket_, string(it, it2).c_str(), send_bytes, MSG_NOSIGNAL);
    }
}


void Client::connectTo(int port) {
    sockaddr_in socket_addr;
    
    bzero(&socket_addr, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    connect(client_socket_, (sockaddr*)(&socket_addr), sizeof(socket_addr));
    //ip_ = inet_ntoa(socket_addr.sin_addr); 
    pollfd poll_set[2];
    poll_set[0].fd = client_socket_;
    poll_set[0].events = POLLIN;
    poll_set[1].fd = 0;
    poll_set[1].events = POLLIN;

    while (true) {
        poll(poll_set, 2, -1);
        if (poll_set[0].revents & POLLIN) {
            int flag = recvMessage();
            if (flag == -1) {
                cout << "lost connection to server" << endl;   
                return;
            }
            if (flag) {
                cout << recv_message_;
                cout.flush();
                recv_message_ = "";
            }
        } else if ((poll_set[0].revents & POLLERR) || (poll_set[0].revents & POLLHUP)) {
            cout << "lost connection to server" << endl;   
            return;
        }
        if (poll_set[1].revents & POLLIN) {
            sendMessage();
        }
    }
}

int main() {
    //std::ios_base::sync_with_stdio(false);
    //cin.tie(nullptr);
    //set_nonblock(0);
    Client client;
    client.connectTo(3100);
    return 0;
}







