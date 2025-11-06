// client.c
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
    // Publicly known numbers (must match the server's)
    long long int P = 23;
    long long int G = 5;

    // Client's private key (a)
    long long int a = 4;
    printf("Client's private key (a): %lld\n", a);

    int client_sock;
    struct sockaddr_in server_addr;

    // Create socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    printf("TCP client socket created.\n");

    // Configure server address
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    printf("Connected to server.\n");

    // 1. Calculate client's public key (A)
    long long int A = power(G, a, P);

    // 2. Send client's public key (A) to server
    send(client_sock, &A, sizeof(A), 0);
    printf("Sent client's public key (A): %lld\n", A);

    // 3. Receive server's public key (B)
    long long int B;
    recv(client_sock, &B, sizeof(B), 0);
    printf("Received server's public key (B): %lld\n", B);

    // 4. Calculate the shared secret key
    long long int shared_secret = power(B, a, P);
    printf("--------------------------------------------\n");
    printf("Shared Secret Key computed by Client: %lld\n", shared_secret);
    printf("--------------------------------------------\n");

    close(client_sock);
    return 0;
}