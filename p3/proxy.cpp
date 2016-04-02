#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <boost/bind.hpp>
#include <utility>
#include <map>
#include <cstdlib>
#include <ctime>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iterator>

#include "proxy.hpp"

using namespace boost::asio;
using boost::algorithm::split;
using boost::is_any_of;
using std::vector;
using std::string;
using std::ifstream;
using std::make_pair;
using std::map;
using std::cerr;
using std::endl;

ServerAddress::ServerAddress(const string &s, int p): ip(s), port(p) {}

ifstream& operator>>(ifstream &stream, ServerAddress &addr) {
    stream >> addr.ip;
    if (!stream) {
        return stream;
    }
    if (addr.ip.back() == ',') {
        addr.ip.pop_back();
    }
    vector<string> split_vec;
    split(split_vec, addr.ip, is_any_of(":"));
    addr.ip = split_vec[0];
    addr.port = boost::lexical_cast<int>(split_vec[1]);
    cerr << addr.ip << ' ' << addr.port << endl;
    return stream;
}

ConfigProxy::ConfigProxy(const char *s) {
    ifstream fin(s);
    cerr << "READ CONFIG FILE " << endl;
    char trash;
    fin >> port >> trash;
    cerr << port << endl;
    ServerAddress cur_server_addr;
    while (fin >> cur_server_addr) {
        server_addrs.push_back(cur_server_addr);
        cerr << "READ\n";
    }
    cerr << "WE ARE READY\n";
    fin.close();
}

ClientBuffer::ClientBuffer() {
    temp_size = 1024;
    temp_send_buf.resize(temp_size);
    temp_recv_buf.resize(temp_size);
    send_size = 0;
    recv_size = 0;
    server_available = true;
    client_available = true;
}

void ClientBuffer::flushTempSendBuf() {
    send_buf.insert(send_buf.end(), temp_send_buf.begin(), temp_send_buf.begin() + recv_size);
}

void ClientBuffer::flushTempRecvBuf() {
    recv_buf.insert(recv_buf.end(), temp_recv_buf.begin(), temp_recv_buf.begin() + send_size);
}

ProxyServer::ProxyServer(const ConfigProxy &config) : acceptor_(service_, ip::tcp::endpoint(ip::tcp::v4(), config.port)) {
    config_ = config;
    srand(time(nullptr));
}

void ProxyServer::run() {
    cerr << "RUN SRV\n";
    socket_ptr listen_sock(new ip::tcp::socket(service_));
    //listen_sock->set_option(ip::tcp::socket::reuse_address(true));

    acceptClient_(listen_sock);
    service_.run();
}

void ProxyServer::acceptClient_(socket_ptr sock) {
    acceptor_.async_accept(*sock, boost::bind(&ProxyServer::acceptClientHandler_, this, sock, _1));
}

void ProxyServer::connectToServer_(ServerAddress &srv_addr, socket_ptr client_sock) {
    ip::tcp::endpoint endpoint(ip::address::from_string(srv_addr.ip), srv_addr.port);
    socket_ptr server_sock(new ip::tcp::socket(service_));
    //listen_sock->set_option(ip::tcp::socket::reuse_address(true));
    server_sock->async_connect(endpoint, boost::bind(&ProxyServer::connectToServerHandler_, this, client_sock, server_sock, _1));
}


void ProxyServer::recvClientMessage_(socket_ptr client_sock, socket_ptr server_sock) {
    cerr << "START_RECV_CLIENT\n";
    client_sock->async_read_some(buffer(clients_[client_sock].temp_send_buf, clients_[client_sock].temp_size), boost::bind(&ProxyServer::recvClientMessageHandler_, this, client_sock, server_sock, _1, _2)); 
}

void ProxyServer::recvServerMessage_(socket_ptr client_sock, socket_ptr server_sock) {
    server_sock->async_read_some(buffer(clients_[client_sock].temp_recv_buf, clients_[client_sock].temp_size), boost::bind(&ProxyServer::recvServerMessageHandler_, this, client_sock, server_sock, _1, _2)); 
}

void ProxyServer::sendClientMessage_(socket_ptr client_sock, socket_ptr server_sock) {
    cerr << "WANT TO SEND " << std::string(clients_[client_sock].send_buf.begin(), 
    clients_[client_sock].send_buf.end()) << std::endl;
    async_write(*server_sock, buffer(clients_[client_sock].send_buf, clients_[client_sock].send_buf.size()), boost::bind(&ProxyServer::sendClientMessageHandler_, this, client_sock, server_sock, _1));
}

void ProxyServer::sendServerMessage_(socket_ptr client_sock, socket_ptr server_sock) {
    async_write(*client_sock, buffer(clients_[client_sock].recv_buf, clients_[client_sock].recv_buf.size()), boost::bind(&ProxyServer::sendServerMessageHandler_, this, client_sock, server_sock, _1));
}



void ProxyServer::acceptClientHandler_(socket_ptr client_sock, const boost::system::error_code &err) {
    if (err) {
        cerr << "ERROR IN ACCEPT: " << err << std::endl; // kostil
        clients_.erase(client_sock);
        client_sock->close();
        return;
    }
    cerr << "ACCEPTED NEW CLIENT\n";
    cerr << "ADD CLIENT TO CLIENTS\n";
    clients_.insert(make_pair(client_sock, ClientBuffer()));
    
    cerr << "CHOOSE SERVER\n";
    ServerAddress srv_addr = chooseServer_();
    cerr << "SERVER " << srv_addr.ip << ' ' << srv_addr.port << '\n';
    connectToServer_(srv_addr, client_sock); //write
   

    socket_ptr sock(new ip::tcp::socket(service_));
    //sock->set_option(ip::tcp::socket::reuse_address(true));
    acceptClient_(sock);
}



void ProxyServer::connectToServerHandler_(socket_ptr client_sock, socket_ptr server_sock, const boost::system::error_code &err) {
    cerr << "CONNECTED TO SERVER\n";
    if (err) {
        cerr << "ERROR IN CONNECT SERVER: CLOSE CONNECTIONS " << err << std::endl; // kostil
        terminateClientServerConnection_(client_sock, server_sock);
        return;
    }
    recvClientMessage_(client_sock, server_sock);
    recvServerMessage_(client_sock, server_sock);
}


void ProxyServer::recvClientMessageHandler_(socket_ptr client_sock, socket_ptr server_sock, const boost::system::error_code &err, size_t recv_size) {
    if (err && err != error::eof) {
        cerr << "ERROR IN RECV CLIENT MESSAGE: CLOSE CONNECTIONS " << err << std::endl; // kostil
        terminateClientServerConnection_(client_sock, server_sock);
        return;
    }
    cerr << "RECIEVED FROM CLIENT " << err.message() << "\n";
    clients_[client_sock].recv_size = recv_size;
    clients_[client_sock].flushTempSendBuf();
    cerr << "SEND BUF: " << client_sock << ' ' << string(clients_[client_sock].temp_send_buf.begin(), clients_[client_sock].temp_send_buf.begin() + recv_size) << std::endl;
    if (err == error::eof) { //kostil
        cerr << "EOF CLIENT!!!\n";
        clients_[client_sock].client_available = false;
        sendClientMessage_(client_sock, server_sock);
    } else {
        recvClientMessage_(client_sock, server_sock);
    }
}


void ProxyServer::recvServerMessageHandler_(socket_ptr client_sock, socket_ptr server_sock, const boost::system::error_code &err, size_t send_size) {
    if (err && err != error::eof) {
        cerr << "ERROR IN RECV SERVER MESSAGE: CLOSE CONNECTIONS " << err << std::endl; // kostil
        terminateClientServerConnection_(client_sock, server_sock);
        return;
    }
    cerr << "RECEIVED FROM SERVER\n";
    clients_[client_sock].send_size = send_size;
    clients_[client_sock].flushTempRecvBuf();
    cerr << "RECV BUF: " << client_sock << ' ' << string(clients_[client_sock].temp_recv_buf.begin(), clients_[client_sock].temp_recv_buf.begin() + send_size) << std::endl;
    if (err == error::eof) {//err == error::eof) //right
        cerr << "EOF SERVER!!!\n";
        clients_[client_sock].server_available = false;
        sendServerMessage_(client_sock, server_sock);
    } else {
        recvServerMessage_(client_sock, server_sock);
    }
}


void ProxyServer::sendClientMessageHandler_(socket_ptr client_sock, socket_ptr server_sock, const boost::system::error_code &err) {
    if (err) {
        cerr << "ERROR IN SEND CLIENT MESSAGE: CLOSE CONNECTIONS " << err << std::endl; // kostil
        terminateClientServerConnection_(client_sock, server_sock);
        return;
    }
    cerr << "SENT TO SERVER\n";
    clients_[client_sock].send_buf.clear();
    if ((!clients_[client_sock].server_available && !clients_[client_sock].client_available && clients_[client_sock].send_buf.empty() && clients_[client_sock].recv_buf.empty()) || !client_sock->available() || !server_sock->available()) {
        terminateClientServerConnection_(client_sock, server_sock);
    }
}


void ProxyServer::sendServerMessageHandler_(socket_ptr client_sock, socket_ptr server_sock, const boost::system::error_code &err) {
    if (err) {
        cerr << "ERROR IN SEND SERVER MESSAGE: CLOSE CONNECTIONS " << err << std::endl; // kostil
        terminateClientServerConnection_(client_sock, server_sock);
        return;
    }
    cerr << "SENT TO CLIENT\n";
    clients_[client_sock].recv_buf.clear(); 
    if ((!clients_[client_sock].server_available && !clients_[client_sock].client_available && clients_[client_sock].send_buf.empty() && clients_[client_sock].recv_buf.empty())  || !client_sock->available() || !server_sock->available()) {
       terminateClientServerConnection_(client_sock, server_sock);
    }
}


ServerAddress ProxyServer::chooseServer_() {
    int cnt_srv = config_.server_addrs.size();
    int cur_srv = rand() % cnt_srv;
    return config_.server_addrs[cur_srv];
}


void ProxyServer::terminateClientServerConnection_(socket_ptr client_sock, socket_ptr server_sock) {
    server_sock->close();
    clients_.erase(client_sock);
    client_sock->close();
}

int main(int argc, char **argv) {
    ConfigProxy config(argv[1]);
    ProxyServer srv(config);
    srv.run();
}
