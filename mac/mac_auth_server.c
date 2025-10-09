#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 5555
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

// --- Whitelist of Authorized MAC Addresses ---
// Add your client's MAC address here for authentication to succeed.
const char *authorized_macs[] = {
    "02:42:76:c2:f4:73",
    "00:15:5d:5d:f3:bd",
    NULL // Sentinel value to mark the end of the array
};

// Function to check if a given MAC address is in the whitelist
int is_authorized(const char *mac) {
    for (int i = 0; authorized_macs[i] != NULL; i++) {
        if (strcmp(mac, authorized_macs[i]) == 0) {
            return 1; // Found
        }
    }
    return 0; // Not found
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 5555
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[*] Server listening on port %d\n", PORT);
    printf("[*] Waiting for a connection...\n");

    // Main server loop to accept and handle connections
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to the next iteration on accept failure
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("\n[*] Accepted connection from %s:%d\n", client_ip, ntohs(address.sin_port));

        // Clear buffer and read the MAC address from the client
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(new_socket, buffer, BUFFER_SIZE - 1); // -1 to leave space for null terminator
        if (valread <= 0) {
            printf("[!] Client disconnected or read error.\n");
            close(new_socket);
            continue;
        }

        printf("[*] Received MAC address: %s\n", buffer);

        // Authenticate the MAC address
        const char *response;
        if (is_authorized(buffer)) {
            response = "200: Authentication Successful";
            printf("[*] MAC address %s is authorized.\n", buffer);
        } else {
            response = "403: Authentication Failed - MAC Address Not Recognized";
            printf("[!] Unauthorized MAC address: %s\n", buffer);
        }

        // Send the response back to the client
        send(new_socket, response, strlen(response), 0);
        
        // Close the connection with the client
        close(new_socket);
        printf("[*] Connection with %s closed.\n", client_ip);
    }

    return 0;
}
