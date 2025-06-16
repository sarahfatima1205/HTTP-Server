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
int main() {
    WSADATA wsa;
    // Initialize Winsock
    //tcp socket creation
    SOCKET server_socket, client_socket;
    sockaddr_in server{}, client{};
    int client_size = sizeof(client);

    if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)); // sometimes if it restarts quickly, it may fail to bind due to address already in use, hence allows reuse of the address

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;
    //bind the socket to the address and port
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    //listen for incoming connections
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Listening on port 8080...\n";
    //accept an incoming connection
    //accept blocks until a client connects
    // accept client
    client_socket = accept(server_socket, (sockaddr*)&client, &client_size);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    std::cout << "Client connected!\n";

    //Read the client's request
    char buffer[4096];
    int bytes_received=recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received>0) {
        buffer[bytes_received] = '\0';  // Null-terminate for safe printing
        std::cout << "Client sent request:\n" << buffer << "\n";
    } else {
        std::cerr << "Failed to read client request\n";
    }

    // respond to the client (same as before)
    const char* http_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
        
    int bytes_sent = send(client_socket, http_response, strlen(http_response), 0);
    if (bytes_sent == SOCKET_ERROR) {
        std::cerr << "Failed to send response\n";
    } else {
        std::cout << "Sent " << bytes_sent << " bytes to client.\n";
    }
    //cleanup
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    return 0;
}