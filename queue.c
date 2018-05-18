// Implementação da fila
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "queue.h"

// Função para criar uma fila com o tamanho especificado
// As filas são inicializadas com 0
struct Queue* createQueue(unsigned capacity) {
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  
    queue->array = (int*) malloc(queue->capacity * sizeof(int));

    return queue;
}

// A fila está cheia quando seu tamanho é igual a sua capacidade
int isFull(struct Queue* queue) {
    return (queue->size == queue->capacity);
}

// A fila está vazia quando seu tamanho é 0
int isEmpty(struct Queue* queue) {
    return (queue->size == 0);
}

// Função para enfileirar; ela altera o último elemento e a quantidade
void enqueue(struct Queue* queue, int item) {
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

// Função para desinfileirar; ela altera o primeiro elemento e a quantidade
int dequeue(struct Queue* queue) {
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;

    return item;
}

// Função para encontrar o primeiro elemento
int front(struct Queue* queue) {
    if (isEmpty(queue))
        return INT_MIN;

    return queue->array[queue->front];
}

// Função para encontrar o último elemento
int rear(struct Queue* queue) {
    if (isEmpty(queue))
        return INT_MIN;

    return queue->array[queue->rear];
}