#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h> 

#define SERVER_PORT 5555
#define BUFFER_SIZE 1024
#define MAC_STR_LEN 18 // "xx:xx:xx:xx:xx:xx\0"

/**
 * @brief Gets the MAC address of a given network interface (Linux specific).
 * * @param interface_name The name of the network interface (e.g., "eth0", "wlan0").
 * @param mac_address_str Buffer to store the resulting MAC address string.
 * @return 0 on success, -1 on failure.
 * * NOTE: This function is specific to Linux systems that expose network
 * information through the /sys filesystem. This is a common but not
 * universal standard. On other systems (like Windows or macOS), a different,
 * platform-specific API would be needed.
 */
int get_mac_address(const char* interface_name, char* mac_address_str) {
    char path[128];
    FILE *fp;

    snprintf(path, sizeof(path), "/sys/class/net/%s/address", interface_name);

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening MAC address file. The interface may not exist or you may lack permissions");
        return -1;
    }

    if (fgets(mac_address_str, MAC_STR_LEN, fp) == NULL) {
        perror("Error reading MAC address from file");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    
    // Remove the trailing newline character, if it exists
    mac_address_str[strcspn(mac_address_str, "\n")] = 0;

    return 0;
}


int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char mac_address[MAC_STR_LEN];
    const char *server_host = "127.0.0.1"; // Change to server IP if not local
    
    // --- Get Local MAC Address ---
    // IMPORTANT: Change "eth0" to your primary network interface name.
    // You can find it by running the `ip addr` or `ifconfig` command in your terminal.
    // Common names include "eth0", "enp0s3", "wlan0", "wlp2s0".
    if (get_mac_address("eth0", mac_address) != 0) {
        fprintf(stderr, "[!] Could not retrieve MAC address for 'eth0'.\n");
        fprintf(stderr, "[!] Please check the interface name and try again.\n");
        exit(EXIT_FAILURE);
    }
    printf("[*] This machine's MAC address is: %s\n", mac_address);

    // Create the socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_host, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    printf("[*] Connecting to server at %s:%d...\n", server_host, SERVER_PORT);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
    printf("[*] Connection successful.\n");

    // Send the MAC address to the server
    send(sock, mac_address, strlen(mac_address), 0);

    // Receive the authentication response from the server
    int valread = read(sock, buffer, BUFFER_SIZE - 1);
    if (valread <= 0) {
        printf("[!] Server closed connection or read error.\n");
    } else {
       buffer[valread] = '\0'; // Null-terminate the received string
       printf("\n--- Server Response ---\n");
       printf("%s\n", buffer);
       printf("-----------------------\n");
    }

    // Clean up the connection
    close(sock);
    printf("[*] Connection closed.\n");
    return 0;
}
