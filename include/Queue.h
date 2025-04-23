#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#define MAX_SIZE 1000

typedef struct{
	int x;
	int y;
}VECTOR2I;

// Defining the Queue structure
typedef struct {
    VECTOR2I r[MAX_SIZE * MAX_SIZE];
    int front;
    int rear;
} Queue;

// Function to initialize the queue
void initializeQueue(Queue* q)
{
    q->front = -1;
    q->rear = 0;
}

// Function to check if the queue is empty
bool isEmpty(Queue* q) { return (q->front == q->rear - 1); }

// Function to check if the queue is full
bool isFull(Queue* q) { return (q->rear == MAX_SIZE); }

// Function to add an element to the queue (Enqueue
// operation)
void enqueue(Queue* q, VECTOR2I* p)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->r[q->rear] = *p;
    q->rear++;
}

// Function to remove an element from the queue (Dequeue
// operation)
void dequeue(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }
    q->front++;
}

// Function to get the element at the front of the queue
// (Peek operation)
VECTOR2I peek(Queue* q)
{
	VECTOR2I x = {-1,-1};
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return x; // return some default value or handle
                   // error differently
    }
    return q->r[q->front + 1];
}




#endif
