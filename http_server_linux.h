#pragma once

#include <netinet/in.h>   // sockaddr_in, in_addr, htons()
#include <string>  
#include <functional>

namespace http {

    class TcpServer{
        public:
            TcpServer(int port);
            ~TcpServer();

        private:
            ssize_t server_socket;
            struct sockaddr_in server_address;

            int startServer();
            void closeServer();
            void handleRequest(ssize_t client_socket, struct sockaddr*client_address, socklen_t* client_addrlen);

        public:            
            int GET(ssize_t client_socket); //deprecated
            int POST(ssize_t client_socket, std::string body); //deprecated
            int send_404(ssize_t client_socket);
        };    
} //namespace http
