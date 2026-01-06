# Blocking HTTP Server

A minimal **blocking HTTP server** implemented in C++ for Linux.  
This project demonstrates socket programming, manual HTTP parsing, and simple route handling — built from first principles.

---

## Features
- 🔒 Blocking I/O model using `accept()` and `recv()`
- 📜 Basic HTTP request parsing (`GET`, `POST`)
- 🛠️ Simple routing system for custom endpoints
- 🐧 Linux implementation (tested on Ubuntu)
- 🎓 Educational focus: clarity and fundamentals over production readiness

---

## Routes

# GET `/`
Returns a plain text response. Used as a basic root endpoint.

# GET `/file`
Downloads `something.txt` from the server.

- Forces file download (`Content-Disposition: attachment`)
- Streams file in 8 KB chunks
- Sets correct `Content-Length`

# POST `/login`
Echoes the request body.

- Responds with `201 Created`
- Response body equals request body

---

## Requirements

- Linux environment
- C++17 or later
- g++ or compatible compiler

---

## Build

```
git clone https://github.com/SonicSegFault/Blocking-http-server.git
cd Blocking-http-server
g++ -std=c++17 server_linux.cpp http_server_linux.cpp http_routing.cpp -o server
```
---