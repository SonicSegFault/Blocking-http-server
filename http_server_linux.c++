#include "http_server_linux.h"
#include "http_routing.h"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <algorithm>

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
        socklen_t client_addrlen = sizeof(client_address);
        if((client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addrlen)) < 0){
            perror("Failed to accept Client"); 
        } else { handleRequest(client_socket, (struct sockaddr*)&client_address, &client_addrlen); }
    }
    return 0;
}

void http::TcpServer::handleRequest(ssize_t client_socket, struct sockaddr* client_address, socklen_t* client_addrlen) {
    char buf[4096];
    std::string buffer;

    bool headers_done = false;
    size_t body_start = 0;
    size_t content_length = 0;

    while (true) {
        ssize_t n = recv(client_socket, buf, sizeof(buf), 0);
        if (n == 0) break;
        if (n < 0) return;

        buffer.append(buf, n);

        // Parse headers when possible
        if (!headers_done) {
            size_t pos = buffer.find("\r\n\r\n");
            if (pos == std::string::npos)
                continue;

            headers_done = true;
            body_start = pos + 4;

            // parse Content-Length (if any)
            auto lowerbuffer = buffer;
            std::transform(lowerbuffer.begin(), lowerbuffer.end(), lowerbuffer.begin(), ::tolower);
            size_t cl = lowerbuffer.find("content-length:");
            if (cl != std::string::npos) {
                size_t end = buffer.find("\r\n", cl);
                std::string len =
                    buffer.substr(cl + 15, end - (cl + 15));
                content_length = std::stoul(len);
            } else {
                content_length = 0;
            }
        }

        if (headers_done) {
            // GET → no body
            if (content_length == 0)
                break;

            // POST → wait for full body
            if (buffer.size() - body_start >= content_length)
                break;
        }
    }
    std::string header = buffer.substr(0, body_start);
    std::string body   = buffer.substr(body_start, content_length);

    std::istringstream iss(header);
    std::string method, path, version;
    iss >> method >> path >> version;

    std::cout << method << "\t" << path << std::endl; 
    if (method == "GET") {
        if(get_response.count(path)) get_response[path](client_socket, body);
        else send_404(client_socket);
    } else if (method == "POST") {
        if(post_response.count(path)) post_response[path](client_socket, body);
        else send_404(client_socket);
    }
    std::cout<< buffer << std::endl;
    close(client_socket);
}

//DEPRECATED
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

//DEPRECATED    
int http::TcpServer::POST(ssize_t client_socket, std::string body){
    std::string response = "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: "
    + std::to_string(body.size()) + "\r\n" "\r\n" + body;
    
    ssize_t responseSize = response.size(), sentSize = 0;
    for(ssize_t i = 0; i<responseSize; i += sentSize){
        sentSize = send(client_socket, response.c_str()+i, responseSize - i, 0);
        if(sentSize <= 0) return 500;
    }
    return 200;
} 

int http::TcpServer::send_404(ssize_t client_socket){
    const char* res =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: close\r\n"
        "\r\n"
        "404 Not Found";
    size_t sentSize = 0;
    for(size_t i = 0; i < sizeof(res); i+=sentSize){
        sentSize = send(client_socket, res+i, sizeof(res)-i, 0);
        if(sentSize <= 0) return 500;
    }
    return 404;
}

void http::TcpServer::closeServer() { 
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
}
