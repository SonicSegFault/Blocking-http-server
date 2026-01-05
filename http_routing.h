#pragma once

#include <functional>
#include <string>
#include <sys/types.h>
#include <unordered_map>

namespace http
{
    using Handler = std::function<void(ssize_t client_socket, std::string&)>;

    extern std::unordered_map<std::string, Handler>get_response;
    extern std::unordered_map<std::string, Handler>post_response;

    void init_routes();

} // namespace http
