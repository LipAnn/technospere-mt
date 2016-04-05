#include <vector>
#include <utility>
#include <string>
#include <fstream>
#include <set>
#include <map>
#include <boost/asio.hpp>

using std::vector;
using std::pair;
using std::string;
using std::set;
using std::map;
using std::ifstream;
using namespace boost::asio;
using socket_ptr = boost::shared_ptr<ip::tcp::socket>;

struct ServerAddress {
    string ip;
    int port;
    ServerAddress(const string&, int);
    ServerAddress() = default;
    friend ifstream& operator>>(ifstream&, ServerAddress&);
};

struct ConfigProxy {
    int port;
    vector<ServerAddress> server_addrs;
    ConfigProxy(const char*);
    ConfigProxy() = default;
};

struct ClientBuffer {
    size_t temp_size;
    vector<char> temp_send_buf;
    vector<char> send_buf;
    vector<char> temp_recv_buf;
    vector<char> recv_buf;
    size_t send_size;
    size_t recv_size;
    bool server_available;
    bool client_available;
    bool server_closed;
    bool client_closed;
    ServerAddress server;
    
    ClientBuffer();

    void flushTempSendBuf();
    void flushTempRecvBuf();
};

class ProxyServer {
private:
    ConfigProxy config_;
    map<socket_ptr, ClientBuffer> clients_;
    io_service service_;
    ip::tcp::acceptor acceptor_;

    void acceptClient_(socket_ptr);
    void connectToServer_(ServerAddress &srv_addr, socket_ptr);
    void recvClientMessage_(socket_ptr, socket_ptr);
    void recvServerMessage_(socket_ptr, socket_ptr);
    void sendClientMessage_(socket_ptr, socket_ptr);
    void sendServerMessage_(socket_ptr, socket_ptr);

    void acceptClientHandler_(socket_ptr, const boost::system::error_code&); 
    void connectToServerHandler_(socket_ptr, socket_ptr, const boost::system::error_code&);
    void recvClientMessageHandler_(socket_ptr, socket_ptr, const boost::system::error_code&, size_t);
    void recvServerMessageHandler_(socket_ptr, socket_ptr, const boost::system::error_code&, size_t);
    void sendClientMessageHandler_(socket_ptr, socket_ptr, const boost::system::error_code&);
    void sendServerMessageHandler_(socket_ptr, socket_ptr, const boost::system::error_code&);
    
    ServerAddress chooseServer_();
    void terminateClientServerConnection_(socket_ptr, socket_ptr);
public:
    ProxyServer(const ConfigProxy&);
    ProxyServer() = default;
    void run();
};  
