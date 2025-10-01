#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define MAX_CLIENTS 5

void *handle_tcp(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[1024] = {0};
    int valread;

    while ((valread = read(client_socket, buffer, 1024)) > 0) {
        printf("Received from TCP client: %s\n", buffer);
        send(client_socket, "Message received", strlen("Message received"), 0);
        memset(buffer, 0, sizeof(buffer));
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int tcp_sock, udp_sock, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    pthread_t tid;

    // Create TCP socket
    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("TCP socket failed");
        exit(EXIT_FAILURE);
    }

    // Set TCP socket options
    if (setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind TCP socket
    if (bind(tcp_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("TCP bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for TCP connections
    if (listen(tcp_sock, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Create UDP socket
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("UDP socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind UDP socket
    if (bind(udp_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("UDP bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    fd_set readfds;
    int max_sd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(tcp_sock, &readfds);
        FD_SET(udp_sock, &readfds);
        max_sd = (tcp_sock > udp_sock) ? tcp_sock : udp_sock;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select error");
        }

        if (FD_ISSET(tcp_sock, &readfds)) {
            if ((new_socket = accept(tcp_sock, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            if (pthread_create(&tid, NULL, handle_tcp, &new_socket) != 0) {
                printf("Failed to create thread\n");
            }
        }

        if (FD_ISSET(udp_sock, &readfds)) {
            int len, n;
            len = sizeof(address);
            n = recvfrom(udp_sock, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &address, &len);
            buffer[n] = '\0';
            printf("Received from UDP client: %s\n", buffer);
            sendto(udp_sock, "Message received", strlen("Message received"), MSG_CONFIRM, (const struct sockaddr *) &address, len);
        }
    }

    return 0;
}
