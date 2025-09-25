#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 65432
#define SERVER_IP "127.0.0.1"

void error_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main() {
    int client_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    // 1. Create a UDP socket
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        error_exit("socket creation failed");
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // Convert IPv4 address from string to network address structure
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        error_exit("Invalid address/ Address not supported");
    }

    printf("Enter message to send to server: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
        error_exit("Failed to read input");
    }
    
    // 2. Send the message to the server
    if (sendto(client_socket, buffer, strlen(buffer), 0,
               (struct sockaddr *)&server_addr, server_addr_len) == -1) {
        error_exit("sendto failed");
    }
    printf("Message sent to server.\n");

    // 3. Receive response from the server
    ssize_t bytes_received = recvfrom(client_socket, buffer, BUFFER_SIZE, 0,
                                     (struct sockaddr *)&server_addr, &server_addr_len);
    if (bytes_received == -1) {
        error_exit("recvfrom failed");
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data
    printf("Received response from server: %s\n", buffer);

    


// 4. Close the socket
    close(client_socket);
    return 0;
}


