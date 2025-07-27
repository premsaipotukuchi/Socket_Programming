#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8081
#define BUFFER_SIZE 1024
#define FRAME_SIZE 64
#define SYN 22

// Function to calculate parity bit
char add_parity_bit(char character) {
    int count = 0;
    for (int i = 0; i < 7; i++) {
        if (character & (1 << i)) count++;
    }
    return (count % 2 == 0) ? (character & 0x7F) : (character | 0x80);
}

// Function to remove parity bit
char remove_parity_bit(char character) {
    return character & 0x7F; // Remove the 8th bit
}

// Function to encode data into frames
void encode(char *input, char *output) {
    int length = strlen(input);
    char *ptr = output;

    for (int i = 0; i < length; i += FRAME_SIZE) {
        *ptr++ = SYN;  // First SYN
        *ptr++ = SYN;  // Second SYN

        int chunk_size = (i + FRAME_SIZE <= length) ? FRAME_SIZE : (length - i);
        *ptr++ = chunk_size; // Length byte

        for (int j = 0; j < chunk_size; j++) {
            *ptr++ = add_parity_bit(input[i + j]); // Add parity bit
        }
    }
    *ptr = '\0';
}

// Function to decode frames into original data
void decode(char *input, char *output) {
    char *ptr = input;
    char *out_ptr = output;

    while (*ptr) {
        if (*ptr == SYN && *(ptr + 1) == SYN) {
            ptr += 2; // Skip SYN characters
        } else {
            break;
        }

        int length = *ptr++; // Length byte
        for (int i = 0; i < length; i++) {
            *out_ptr++ = remove_parity_bit(*ptr++); // Remove parity bit
        }
    }
    *out_ptr = '\0';
}

// Process encoding/decoding requests
void process_request(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};

    read(client_socket, buffer, BUFFER_SIZE);

    if (strncmp(buffer, "ENCODE ", 7) == 0) {
        encode(buffer + 7, response);
    } else if (strncmp(buffer, "DECODE ", 7) == 0) {
        decode(buffer + 7, response);
    } else {
        snprintf(response, sizeof(response), "Invalid request type");
    }

    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Helper is listening on port %d\n", PORT);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        process_request(client_socket);
    }

    close(server_fd);
    return 0;
}
