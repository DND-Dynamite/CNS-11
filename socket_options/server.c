#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 1024

void configure_socket_options(int server_fd) {
    int opt = 1;
    struct timeval timeout;
    
    // SO_REUSEADDR - Allow reuse of local addresses
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        exit(EXIT_FAILURE);
    }
    printf("SO_REUSEADDR enabled\n");
    
    // SO_REUSEPORT - Allow reuse of local ports (Linux specific)
    #ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEPORT failed");
        // Continue anyway as this might not be critical
    } else {
        printf("SO_REUSEPORT enabled\n");
    }
    #endif
    
    // SO_RCVTIMEO - Set receive timeout
    timeout.tv_sec = 5;  // 5 seconds
    timeout.tv_usec = 0;
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt SO_RCVTIMEO failed");
    } else {
        printf("SO_RCVTIMEO set to 5 seconds\n");
    }
    
    // SO_SNDTIMEO - Set send timeout
    timeout.tv_sec = 5;  // 5 seconds
    timeout.tv_usec = 0;
    if (setsockopt(server_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt SO_SNDTIMEO failed");
    } else {
        printf("SO_SNDTIMEO set to 5 seconds\n");
    }
    
    // TCP_NODELAY - Disable Nagle's algorithm for lower latency
    opt = 1;
    if (setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        perror("setsockopt TCP_NODELAY failed");
    } else {
        printf("TCP_NODELAY enabled\n");
    }
}

void handle_client(int client_fd, struct sockaddr_in *client_addr) {
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int bytes_received;
    
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Client connected from %s:%d\n", client_ip, ntohs(client_addr->sin_port));
    
    while (1) {
        // Clear buffer
        memset(buffer, 0, BUFFER_SIZE);
        
        // Receive data from client
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Received from client: %s", buffer);
            
            // Check for exit command
            if (strncmp(buffer, "exit", 4) == 0) {
                printf("Client requested disconnect\n");
                break;
            }
            
            // Send response back to client (use a safe bounded format to avoid truncation warnings)
            char response[BUFFER_SIZE];
            const char *prefix = "Server received: ";
            size_t prefix_len = strlen(prefix);
            /* Reserve space for null terminator */
            if (prefix_len >= sizeof(response)) {
                /* Shouldn't happen for current prefix, but guard anyway */
                response[0] = '\0';
            } else {
                /* Compute max number of chars we can copy from buffer */
                int max_copy = (int)(sizeof(response) - prefix_len - 1);
                if (max_copy < 0) max_copy = 0;
                /* Use precision to limit how much of buffer is inserted */
                int written = snprintf(response, sizeof(response), "%s%.*s", prefix, max_copy, buffer);
                /* snprintf returns the number of bytes that would have been written (excluding NUL)
                   We don't need to check for truncation here; response is always NUL-terminated by snprintf */
                (void)written;
            }
            if (send(client_fd, response, strlen(response), 0) < 0) {
                perror("send failed");
                break;
            }
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Receive timeout occurred\n");
                // Send timeout message to client
                char *timeout_msg = "Server timeout - no data received\n";
                send(client_fd, timeout_msg, strlen(timeout_msg), 0);
                break;
            } else {
                perror("recv failed");
                break;
            }
        }
    }
    
    close(client_fd);
    printf("Connection with client %s:%d closed\n", client_ip, ntohs(client_addr->sin_port));
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created\n");
    
    // Configure socket options
    configure_socket_options(server_fd);
    
    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to port %d\n", PORT);
    
    // Listen for connections
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);
    
    // Handle SIGCHLD to avoid zombie processes
    signal(SIGCHLD, SIG_IGN);
    
    while (1) {
        // Accept incoming connection
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }
        
        // Fork to handle client in separate process
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(server_fd);  // Close server socket in child
            handle_client(client_fd, &client_addr);
            exit(0);
        } else if (pid > 0) {
            // Parent process
            close(client_fd);  // Close client socket in parent
        } else {
            perror("fork failed");
            close(client_fd);
        }
    }
    
    close(server_fd);
    return 0;
}