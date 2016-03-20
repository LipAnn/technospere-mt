#include <fcntl.h>
#include <string>
#include <map>
#include <set>

using std::string;
using std::map;
using std::set;

class Server {
private:
    int listen_socket_;
    int max_buf_size_;
    map<size_t, string> message_buf_;
    set<int> clients_;
public:
    Server(int);  // add third pearmeter
    ~Server();
    void terminateConnection(int, int);
    int recvMessage(int, int);
    void sendMessage(int);
    void run();
};

class Client {
private:
    int client_socket_;
    int max_buf_size_;
    string recv_message_;
    string ip_;
public:
    Client();
    ~Client();
    int recvMessage();
    void sendMessage();
    void connectTo(int);
};


int set_nonblock(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
