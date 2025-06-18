#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib") // Winsock library

// terms:
// 1. WAStartup() initializes the Win-sockets api
// 2. sockets are endpoints for sending and receiving data
// 3. socket() creates a TCP socket 
// 4. bind()- binds the socket to an IP address and port
// 5. listen()- prepares the socket to accept incoming connections
// 6. accept()- accepts an incoming connection request, blocks until a client connections
// 7. closesocket()-frees up the socket
// 8. WSACleanup()- cleans up the Win-sockets api, releases winsock resources!


// Task-1: start a TCP socket server that listens on port 8080, wait for a client to connect, accept the connection and print a messg thrn shut down the server.
//teast-2: adding a server response when it gets connected
//task-3: parsing the webserv req
//task-4: upgrade to handle multiple clients, running til true->
//each client is accepted, req is read, response is sent, then the client is closed, loop conntinues!

int main() {
    WSADATA wsa;
    SOCKET server_socket;
    sockaddr_in server{};
    //initialize winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Step 2: create TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // Step 3: allow address reuse (helpful during quick restarts)
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // Step 4: define server address and bind to port 8080
    server.sin_family = AF_INET;
    server.sin_port = htons(8080); // host to network short
    server.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Step 5: start listening for incoming connections
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Listening on port 8080...\n";

    // Step 6: loop to accept multiple client connections
    while (true) {
        SOCKET client_socket;
        sockaddr_in client{};
        int client_size = sizeof(client);

        // Accept a client connection (blocking)
        client_socket = accept(server_socket, (sockaddr*)&client, &client_size);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue; // skip to next client
        }

        std::cout << "\nClient connected!\n";

        // Step 7: receive HTTP request
        char buffer[4096]; // 4KB buffer for incoming request
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cerr << "Failed to read client request\n";
            closesocket(client_socket);
            continue;
        }

        // Null-terminate for safe string printing
        buffer[bytes_received] = '\0';
        std::string request(buffer);
        std::cout << "=== Full HTTP Request ===\n" << request << "\n";

        // Step 8: parse method and path from request line (e.g., GET /hello HTTP/1.1)
        std::istringstream request_stream(request);
        std::string method, path;
        request_stream >> method >> path;

        std::cout << "Requested method: " << method << "\n";
        std::cout << "Requested path: " << path << "\n";

        // Step 9: build appropriate response based on path
        std::string body = get_response_for_path(path);
        bool is_404 = (body == "404 Not Found");

        std::string status_line = is_404 ? "HTTP/1.1 404 Not Found" : "HTTP/1.1 200 OK";

        std::string response =
            status_line + "\r\n" +
            "Content-Length: " + std::to_string(body.length()) + "\r\n" +
            "Content-Type: text/plain\r\n" +
            "\r\n" +
            body;

        // Step 10: send response to the client
        int bytes_sent = send(client_socket, response.c_str(), response.size(), 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "Failed to send response\n";
        } else {
            std::cout << "Sent " << bytes_sent << " bytes to client.\n";
        }

        // Step 11: close client socket, then loop back to accept next client
        closesocket(client_socket);
    }

    // Cleanup (though this part won't be reached unless you break the loop)
    closesocket(server_socket);
    WSACleanup();
    return 0;
}