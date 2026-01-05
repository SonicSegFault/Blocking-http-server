#include "http_server_linux.h"
#include "http_routing.h"

int main() {
    using namespace http;
    
    http::init_routes();
    TcpServer server(8000);

    return 0;
}
