#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void configure_client_socket(int sockfd) {
    int opt = 1;
    struct timeval timeout;
    
    // SO_RCVTIMEO - Set receive timeout
    timeout.tv_sec = 10;  // 10 seconds
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt SO_RCVTIMEO failed");
    } else {
        printf("Client SO_RCVTIMEO set to 10 seconds\n");
    }
    
    // SO_SNDTIMEO - Set send timeout
    timeout.tv_sec = 10;  // 10 seconds
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt SO_SNDTIMEO failed");
    } else {
        printf("Client SO_SNDTIMEO set to 10 seconds\n");
    }
    
    // TCP_NODELAY - Disable Nagle's algorithm
    opt = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        perror("setsockopt TCP_NODELAY failed");
    } else {
        printf("TCP_NODELAY enabled\n");
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    
    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Client socket created\n");
    
    // Configure client socket options
    configure_client_socket(sockfd);
    
    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server %s:%d\n", SERVER_IP, PORT);
    
    printf("Type messages to send to server (type 'exit' to quit):\n");
    
    while (1) {
        printf("Client: ");
        fflush(stdout);
        
        // Read input from user
        if (fgets(message, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        
        // Send message to server
        if (send(sockfd, message, strlen(message), 0) < 0) {
            perror("send failed");
            break;
        }
        
        // Check if user wants to exit
        if (strncmp(message, "exit", 4) == 0) {
            printf("Disconnecting from server...\n");
            break;
        }
        
        // Clear buffer
        memset(buffer, 0, BUFFER_SIZE);
        
        // Receive response from server
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Server: %s", buffer);
        } else if (bytes_received == 0) {
            printf("Server disconnected\n");
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Receive timeout - no response from server\n");
            } else {
                perror("recv failed");
                break;
            }
        }
    }
    
    close(sockfd);
    printf("Connection closed\n");
    
    return 0;
}