// File: server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 1. Create a socket file descriptor
    // AF_INET: IPv4, SOCK_STREAM: TCP, 0: IP protocol
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Optional: Helps in reusing the address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 2. Bind the socket to the network address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    address.sin_port = htons(PORT); // Convert port to network byte order

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen for incoming connections
    // The second argument is the backlog, the max number of pending connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // 4. Accept an incoming connection 🤝
    // This is a blocking call. It waits for a client to connect.
    // It creates a new socket (`new_socket`) for this specific connection.
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted.\n");

    // 5. Communicate with the client (read and write)
    int valread = read(new_socket, buffer, BUFFER_SIZE);
    printf("Client says: %s\n", buffer);

    char *hello = "Hello from server";
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    // 6. Close the sockets
    close(new_socket); // Close the connection with the client
    close(server_fd);  // Close the listening socket

    return 0;
}
