#include "http_server_linux.h"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <sstream>

http::TcpServer::TcpServer(int port): server_socket(-1) {
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    startServer();
}

http::TcpServer::~TcpServer() { closeServer(); }

int http::TcpServer::startServer() {
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("FD Creation Failed"); return -1;
    }
    //make it reuseable after closing
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { perror("setsockopt error"); return -1; }
    if(bind(server_socket, (const sockaddr*)&server_address, sizeof(server_address)) < 0){ perror("bind error"); return -1; }
    if(listen(server_socket, 5) < 0) { perror("Server not listening"); return -1; }

    while(true){
        ssize_t client_socket;
        sockaddr_in client_address;
        socklen_t client_addrlen;
        if((client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addrlen)) < 0){
            perror("Failed to accept Client"); 
        } else { handleRequest(client_socket, (struct sockaddr*)&client_address, &client_addrlen); }
    }

    return 0;
}

void http::TcpServer::handleRequest(ssize_t client_socket, struct sockaddr* client_address, socklen_t* client_addrlen) {
    char BUFFER[4096] = {};
    std::string httpheader;
    while(httpheader.find("\r\n\r\n") == std::string::npos){
        int headersize;
        if((headersize = recv(client_socket, BUFFER, sizeof(BUFFER), 0)) < 0 ) {
            perror("Connection lost"); return;
        }
        httpheader.append(BUFFER, headersize);
    }
    
    std::istringstream iss(httpheader);
    std::string method, path, version;
    iss >> method >> path >> version;

    if (method == "GET") { GET(); }
    else if (method == "POST") { POST(); }

}

void http::TcpServer::closeServer() { 
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
}
