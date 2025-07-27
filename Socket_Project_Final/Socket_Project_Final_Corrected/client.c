#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define HELPER_PORT 8081
#define BUFFER_SIZE 1024

// Function to read input from a file
void read_file(const char *filename, char *data) {
    
    FILE *fp = fopen(filename, "r");  
 	fseek(fp, 0, SEEK_END);		 //Find end of the file 
 	long length = ftell(fp);    //find file data length 
 	fseek(fp, 0, SEEK_SET);   //sets the cursor 
 	fread(data, length, 1, fp); //read data
 	fclose(fp);
 	printf("%s",data);
}

// Function to save decoded results to a file
void save_to_file(const char *filename, const char *data) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s\n", data);
    fclose(file);
}

// Function to interact with the helper for encoding/decoding
void interact_with_helper(const char *request, char *response) {
    int sock;
    struct sockaddr_in helper_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    helper_addr.sin_family = AF_INET;
    helper_addr.sin_port = htons(HELPER_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &helper_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&helper_addr, sizeof(helper_addr)) < 0) {
        perror("Connection to helper failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    send(sock, request, strlen(request), 0);
    read(sock, response, BUFFER_SIZE);
    close(sock);
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char input_data[BUFFER_SIZE] = {0};
    char encoded_data[BUFFER_SIZE] = {0};
    char buffer[BUFFER_SIZE] = {0};
    char decoded_data[BUFFER_SIZE] = {0};

    // Read input from file
    const char *input_filename = "input.txt";
    read_file(input_filename, input_data);

    // Encode input data using helper
    char helper_request[BUFFER_SIZE];

    snprintf(helper_request, sizeof(helper_request), "ENCODE %s", input_data);
    printf("\n%s",helper_request);
    interact_with_helper(helper_request, encoded_data);
    printf("Encoded Input Data: %s\n", encoded_data);

    // Create socket to communicate with the server
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection to server failed");
        return -1;
    }

    // Send encoded data to the server
    send(sock, encoded_data, strlen(encoded_data), 0);
    printf("Encoded Data Sent to Server: %s\n", encoded_data);

    // Receive encoded vowel counts from the server
    read(sock, buffer, BUFFER_SIZE);
    printf("Encoded Vowel Counts Received: %s\n", buffer);

    // Decode the vowel counts using helper
    snprintf(helper_request, sizeof(helper_request), "DECODE %s", buffer);
    interact_with_helper(helper_request, decoded_data);
    printf("Decoded Vowel Counts: %s\n", decoded_data);

    // Save the decoded vowel counts to a file
    const char *output_filename = "receivedVowelCount.txt";
    save_to_file(output_filename, decoded_data);

    // Close the socket
    close(sock);

    return 0;
}
