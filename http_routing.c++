#include "http_routing.h"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <algorithm>

std::unordered_map<std::string, http::Handler> http::get_response;
std::unordered_map<std::string, http::Handler> http::post_response;

void http::init_routes() {
    get_response["/"] = [](ssize_t client_socket, std::string &body){
        std::string responseBody = "Hello World!";
        std::string response = "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: " 
        + std::to_string(responseBody.size()) + "\r\n" "\r\n"
        + responseBody;
        size_t sentSize = 0;
        for(size_t i = 0; i < response.size(); i+=sentSize){
            sentSize = send(client_socket, response.c_str()+i, response.size()-i, 0);
            if(sentSize <= 0) {perror("response not sent"); return;}
        }
        return;
    };

    get_response["/about"] = [](ssize_t client_socket, std::string &body){
        std::string response = "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: " 
        + std::to_string(13) + "\r\n" "\r\n"
        + "this is about";
        size_t sentSize = 0;
        for(size_t i = 0; i < response.size(); i+=sentSize){
            sentSize = send(client_socket, response.c_str()+i, response.size()-i, 0);
            if(sentSize <= 0) {perror("response not sent"); return;}
        }
        return;
    };

    post_response["/login"] = [](ssize_t client_socket, std::string &body){
        std::string response = "HTTP/1.1 201 Created\r\n" "Content-Type: text/plain\r\n" "Content-Length: "
        + std::to_string(body.size()) + "\r\n" "\r\n" + body;
    
        ssize_t responseSize = response.size(), sentSize = 0;
        for(ssize_t i = 0; i<responseSize; i += sentSize){
            sentSize = send(client_socket, response.c_str()+i, responseSize - i, 0);
            if(sentSize <= 0) {perror("response not sent"); return;}
        }
        return;
    };
}
