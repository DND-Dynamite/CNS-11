#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 65432

void error_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main() {
    int server_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 1. Create a UDP socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        error_exit("socket creation failed");
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    server_addr.sin_port = htons(SERVER_PORT);

    // 2. Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error_exit("bind failed");
    }

    printf("UDP Server is listening on port %d...\n", SERVER_PORT);

    while (1) {
        // 3. Receive data from a client
        ssize_t bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE, 0,
                                         (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received == -1) {
            perror("recvfrom failed");
            continue;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received message from %s:%d: %s\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        // 4. Prepare and send a response back to the client
        const char *response = "Hello, client! I received your message.";
        if (sendto(server_socket, response, strlen(response), 0,
                   (struct sockaddr *)&client_addr, client_addr_len) == -1) {
            perror("sendto failed");
        }
    }

    // 5. Close the socket (This part is unreachable in the current loop)
    close(server_socket);
    return 0;
}

