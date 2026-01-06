#include "http_routing.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include <fcntl.h> 

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

    get_response["/file"] = [](ssize_t client_socket, std::string &body){
        int file_des = open("something.txt", O_RDONLY); 
        if(file_des == -1) { perror("File Error"); return; }
        off_t file_size = lseek(file_des, 0, SEEK_END);
        lseek(file_des, 0, SEEK_SET);
        if(file_size == -1) { close(file_des), perror("File Error"); return; }
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Length: " + std::to_string(file_size) + "\r\n"
            "Content-Disposition: attachment; filename=\"" + "something.txt" + "\"\r\n"
            "Connection: close\r\n"
            "\r\n";
        size_t sentSize{};
        for(size_t i{}; i<size(response); i+=sentSize){
            sentSize = send(client_socket, response.c_str()+i, size(response)-i, 0);
            if(sentSize < 0) { close(file_des), perror("File not sent"); return; }
        }

        char body_buffer[8192];
        ssize_t readSize{};
        while((readSize = read(file_des, body_buffer, sizeof(body_buffer))) > 0){
            ssize_t sentSize{};
            for(ssize_t i{}; i < readSize; i+=sentSize){
                sentSize = send(client_socket, body_buffer+i, readSize-i, 0);
                if(sentSize < 0) { close(file_des), perror("Send failed"); return; }
            }
        }
        close(file_des);
        return;
    };

    post_response["/login"] = [](ssize_t client_socket, std::string &body){
        std::string response = "HTTP/1.1 201 Created\r\n" "Content-Type: text/plain\r\n" "Content-Length: "
        + std::to_string(body.size()) + "\r\n" "\r\n" + body;
    
        ssize_t responseSize = response.size(), sentSize = 0;
        for(ssize_t i{}; i<responseSize; i += sentSize){
            sentSize = send(client_socket, response.c_str()+i, responseSize - i, 0);
            if(sentSize <= 0) {perror("response not sent"); return;}
        }
        return;
    };
}
