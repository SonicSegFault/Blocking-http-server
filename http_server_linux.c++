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
    std::string httprequest;
    ssize_t requestSize;
    while((requestSize = recv(client_socket, BUFFER, sizeof(BUFFER), 0)) > 0){
        httprequest.append(BUFFER, requestSize);
    }

    size_t pos = httprequest.find("Content-Length:");
    size_t end = httprequest.find("\r\n", pos);
    std::string strlen = httprequest.substr((pos+15), end-(pos+15));
    size_t contentlen = std::stoi(strlen);
    std::string httpbody;

    size_t headerEndpoint = httprequest.find("\r\n\r\n");
    headerEndpoint = headerEndpoint + 4;

    httpbody = httprequest.substr(headerEndpoint, contentlen);

    std::istringstream iss(httprequest);
    std::string method, path, version;
    iss >> method >> path >> version;

    int statusCode = 500;
    if (method == "GET") { 
        statusCode = GET(client_socket);
    }
    else if (method == "POST") { 
        statusCode = POST(client_socket, httpbody);
    }
}

int http::TcpServer::GET(ssize_t client_socket) {
    std::string body = "Hello, world!";
    std::string response = "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: " 
    + std::to_string(body.size()) + "\r\n" "\r\n"
    + body;
    
    ssize_t responseSize = response.size(), sentSize = 0;
    for(ssize_t i = 0; i<responseSize; i+= sentSize){
       sentSize = send(client_socket, response.c_str()+i, responseSize - i, 0);
       if (sentSize <= 0) { return 500; }
    }
    return 200;
}

int http::TcpServer::POST(ssize_t client_socket, std::string body){

} 

void http::TcpServer::closeServer() { 
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
}
