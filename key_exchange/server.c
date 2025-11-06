// server.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

// Function to compute (base^exp) % mod
long long int power(long long int base, long long int exp, long long int mod) {
    long long int res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return res;
}

int main() {
    // Publicly known numbers
    long long int P = 23; // A prime number
    long long int G = 5;  // A primitive root modulo P

    // Server's private key (b)
    long long int b = 3;
    printf("Server's private key (b): %lld\n", b);

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    printf("TCP server socket created.\n");

    // Configure server address
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind socket to the address
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    printf("Bind to port 8080.\n");

    // Listen for connections
    listen(server_sock, 5);
    printf("Listening...\n");

    addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
    printf("Client connected.\n");

    // 1. Calculate server's public key (B)
    long long int B = power(G, b, P);

    // 2. Receive client's public key (A)
    long long int A;
    recv(client_sock, &A, sizeof(A), 0);
    printf("Received client's public key (A): %lld\n", A);

    // 3. Send server's public key (B) to client
    send(client_sock, &B, sizeof(B), 0);
    printf("Sent server's public key (B): %lld\n", B);

    // 4. Calculate the shared secret key
    long long int shared_secret = power(A, b, P);
    printf("--------------------------------------------\n");
    printf("Shared Secret Key computed by Server: %lld\n", shared_secret);
    printf("--------------------------------------------\n");


    close(client_sock);
    close(server_sock);
    return 0;
}