#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 10

// Structure for the shared queue
typedef struct {
    int items[QUEUE_SIZE];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Queue;

// Initialize the shared queue
void initQueue(Queue* queue) {
    queue->front = -1;
    queue->rear = -1;
    queue->count = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

// Check if the shared queue is empty
int isQueueEmpty(Queue* queue) {
    return queue->count == 0;
}

// Check if the shared queue is full
int isQueueFull(Queue* queue) {
    return queue->count == QUEUE_SIZE;
}

// Add an item to the shared queue
void enqueue(Queue* queue, int item) {
    pthread_mutex_lock(&queue->mutex);

    while (isQueueFull(queue)) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (isQueueEmpty(queue)) {
        queue->front = 0;
    }

    queue->rear = (queue->rear + 1) % QUEUE_SIZE;
    queue->items[queue->rear] = item;
    queue->count++;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

// Remove an item from the shared queue
int dequeue(Queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    while (isQueueEmpty(queue)) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    int item = queue->items[queue->front];

    if (queue->front == queue->rear) {
        queue->front = -1;
        queue->rear = -1;
    } else {
        queue->front = (queue->front + 1) % QUEUE_SIZE;
    }

    queue->count--;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);

    return item;
}

// Structure for thread arguments
typedef struct {
    int id;
    Queue* queue;
} ThreadArgs;

// Function executed by worker threads
void* workerThread(void* arg) {
    ThreadArgs* threadArgs = (ThreadArgs*)arg;
    int threadId = threadArgs->id;
    Queue* queue = threadArgs->queue;

    printf("Worker thread %d started.\n", threadId);

    // Process data from the shared queue
    while (!isQueueEmpty(queue)) {
        pthread_mutex_lock(&queue->mutex);

        while (isQueueEmpty(queue)) {
            pthread_cond_wait(&queue->cond, &queue->mutex);
        }

        int item = dequeue(queue);

        pthread_cond_signal(&queue->cond);
        pthread_mutex_unlock(&queue->mutex);

        printf("Worker thread %d processed item: %d\n", threadId, item);

        // Simulate data processing
        // Replace this with your own processing logic
        usleep(1000000); // Delay for 1 second (1,000,000 microseconds)
    }

    printf("Worker thread %d finished.\n", threadId);

    pthread_exit(NULL);
}

int main() {
    Queue queue;
    initQueue(&queue);

    // Enqueue items into the shared queue
    for (int i = 1; i <= QUEUE_SIZE; i++) {
        enqueue(&queue, i);
    }

    // Create worker threads
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    ThreadArgs threadArgs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        threadArgs[i].id = i + 1;
        threadArgs[i].queue = &queue;
        pthread_create(&threads[i], NULL, workerThread, &threadArgs[i]);
    }

    // Wait for worker threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
