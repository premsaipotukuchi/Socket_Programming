#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define HELPER_PORT 8081
#define BUFFER_SIZE 1024
#define QUEUE_SIZE 5

typedef struct {
    char *queue[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} Queue;

// Global variables
int vowel_count[5] = {0}; // a, e, i, o, u

// Initialize a queue
void init_queue(Queue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

// Enqueue a string
void enqueue(Queue *q, char *str) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == QUEUE_SIZE) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    q->queue[q->tail] = strdup(str);
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

// Dequeue a string
char *dequeue(Queue *q) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }
    char *str = q->queue[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return str;
}

// Helper interaction for decoding
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

void write_vowel_counts_to_file() {
    FILE *file = fopen("vowelCount.txt", "w");
    if (!file) {
        perror("File creation failed");
        return;
    }

    fprintf(file, "Totals\n");
    fprintf(file, "Letter aA : %d\n", vowel_count[0]);
    fprintf(file, "Letter eE : %d\n", vowel_count[1]);
    fprintf(file, "Letter iI : %d\n", vowel_count[2]);
    fprintf(file, "Letter oO : %d\n", vowel_count[3]);
    fprintf(file, "Letter uU : %d\n", vowel_count[4]);

    fclose(file);
}

void send_vowel_counts_to_client(int client_socket) {
    char count_string[BUFFER_SIZE];
    snprintf(count_string, sizeof(count_string), "A=%d, E=%d, I=%d, O=%d, U=%d",
             vowel_count[0], vowel_count[1], vowel_count[2], vowel_count[3], vowel_count[4]);

    // Use helper to encode the count string into frames
    char helper_request[BUFFER_SIZE];
    char encoded_data[BUFFER_SIZE];
    snprintf(helper_request, sizeof(helper_request), "ENCODE %s", count_string);
    interact_with_helper(helper_request, encoded_data);

    // Send the encoded frames to the client
    send(client_socket, encoded_data, strlen(encoded_data), 0);
}

// Thread functions for each vowel
void *charA_thread(void *arg) {
    Queue *queues = (Queue *)arg;
    Queue *input_queue = &queues[0];
    Queue *output_queue = &queues[1];
    char *data;
    while ((data = dequeue(input_queue)) != NULL) {
        for (int i = 0; data[i] != '\0'; i++) {
            if (data[i] == 'a' || data[i] == 'A') vowel_count[0]++;
        }
        enqueue(output_queue, data); // Pass data to the next thread
        free(data);
    }
    pthread_exit(NULL);
}

void *charE_thread(void *arg) {
    Queue *queues = (Queue *)arg;
    Queue *input_queue = &queues[1];
    Queue *output_queue = &queues[2];
    char *data;
    while ((data = dequeue(input_queue)) != NULL) {
        for (int i = 0; data[i] != '\0'; i++) {
            if (data[i] == 'e' || data[i] == 'E') vowel_count[1]++;
        }
        enqueue(output_queue, data); // Pass data to the next thread
        free(data);
    }
    pthread_exit(NULL);
}

void *charI_thread(void *arg) {
    Queue *queues = (Queue *)arg;
    Queue *input_queue = &queues[2];
    Queue *output_queue = &queues[3];
    char *data;
    while ((data = dequeue(input_queue)) != NULL) {
        for (int i = 0; data[i] != '\0'; i++) {
            if (data[i] == 'i' || data[i] == 'I') vowel_count[2]++;
        }
        enqueue(output_queue, data); // Pass data to the next thread
        free(data);
    }
    pthread_exit(NULL);
}

void *charO_thread(void *arg) {
    Queue *queues = (Queue *)arg;
    Queue *input_queue = &queues[3];
    Queue *output_queue = &queues[4];
    char *data;
    while ((data = dequeue(input_queue)) != NULL) {
        for (int i = 0; data[i] != '\0'; i++) {
            if (data[i] == 'o' || data[i] == 'O') vowel_count[3]++;
        }
        enqueue(output_queue, data); // Pass data to the next thread
        free(data);
    }
    pthread_exit(NULL);
}

void *charU_thread(void *arg) {
    Queue *queues = (Queue *)arg;
    Queue *input_queue = &queues[4];
    char *data;
    while ((data = dequeue(input_queue)) != NULL) {
        for (int i = 0; data[i] != '\0'; i++) {
            if (data[i] == 'u' || data[i] == 'U') vowel_count[4]++;
        }
        free(data); // No next thread, so just free the data
    }
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0}, decoded_data[BUFFER_SIZE] = {0};

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
    printf("Server is listening on port %d\n", PORT);

    // Initialize queues
    Queue queues[5];
    for (int i = 0; i < 5; i++) {
        init_queue(&queues[i]);
    }

    // Create threads
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, charA_thread, queues);
    pthread_create(&threads[1], NULL, charE_thread, queues);
    pthread_create(&threads[2], NULL, charI_thread, queues);
    pthread_create(&threads[3], NULL, charO_thread, queues);
    pthread_create(&threads[4], NULL, charU_thread, queues);

    while (1) {
        printf("Waiting for client connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        read(new_socket, buffer, BUFFER_SIZE);

        // Interact with helper to decode data
        char helper_request[BUFFER_SIZE];
        snprintf(helper_request, sizeof(helper_request), "DECODE %s", buffer);
        interact_with_helper(helper_request, decoded_data);

        enqueue(&queues[0], decoded_data); // Start processing with charA_thread
        
        sleep(1);
        write_vowel_counts_to_file();
        send_vowel_counts_to_client(new_socket);
        
        // Print vowel counts
        printf("Final Vowel Counts: A=%d, E=%d, I=%d, O=%d, U=%d\n",
               vowel_count[0], vowel_count[1], vowel_count[2], vowel_count[3], vowel_count[4]);

        // Reset counts
        memset(buffer, 0, BUFFER_SIZE);
        memset(decoded_data, 0, BUFFER_SIZE);
        memset(vowel_count, 0, sizeof(vowel_count));

        close(new_socket);
        
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    close(server_fd);
    return 0;
}
