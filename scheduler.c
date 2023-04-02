#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Created enum to define each state.
typedef enum
{
    RUN,
    WAIT,
    READY
} State;

// Basic Node structure
typedef struct Node
{
    int jobId;
    int processingTime;
    State state;
    struct Node *next;
} Node;

// Linked List structure
typedef struct LinkedList
{
    Node *head;
    int size;
} LinkedList;

// Linked list
void initLinkedList(LinkedList *list)
{
    list->head = NULL;
    list->size = 0;
}

// Create a new node
Node *createNode(int jobId, int processingTime)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->jobId = jobId;
    newNode->processingTime = processingTime;
    newNode->state = RUN;
    newNode->next = NULL;
    return newNode;
}

// Add a new node to the end of the list
void addNode(LinkedList *list, int jobId, int processingTime)
{
    Node *newNode = createNode(jobId, processingTime);

    if (list->head == NULL)
    {
        list->head = newNode;
    }
    else
    {
        Node *current = list->head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newNode;
    }

    list->size++;
}

void *processScheduler(void *arg)
{
    LinkedList *list = (LinkedList *)arg;

    while (true)
    {
        Node *current = list->head;
        while (current != NULL)
        {
            if (current->state == WAIT && current->processingTime <= 0)
            {
                current->state = READY;
                // Condition to change state from WAIT to RUN
                if (rand() % 2 == 0)
                {
                    current->state = RUN;
                }
            }
            else if (current->state == RUN && current->processingTime <= 0)
            {
                current->state = READY;
            }

            current = current->next;
        }
        sleep(1);
    }

    return NULL;
}

void simulateIO(LinkedList *list, int jobId)
{
    Node *current = list->head;
    while (current != NULL)
    {
        if (current->jobId == jobId)
        {
            current->state = WAIT;
            break;
        }
        current = current->next;
    }
}

void fcfs(LinkedList *list)
{
    int waitingTime = 0;
    int turnaroundTime = 0;

    printf("JobID  Waiting Time  Turnaround Time\n");
    printf("------------------------------------\n");

    Node *current = list->head;
    while (current != NULL)
    {
        if (current->state == RUN)
        {
            turnaroundTime = waitingTime + current->processingTime;

            printf("%-5d  %-12d  %-15d\n", current->jobId, waitingTime, turnaroundTime);

            waitingTime = turnaroundTime;
            current->processingTime = 0;
        }
        current = current->next;
    }
}

int main()
{
    srand(time(NULL));
    LinkedList list;
    initLinkedList(&list);

    addNode(&list, 1, 5);
    addNode(&list, 2, 8);
    addNode(&list, 3, 12);

    pthread_t schedulerThread;
    pthread_create(&schedulerThread, NULL, processScheduler, &list);

    // Simulate I/O request for JobID 2
    sleep(2);
    simulateIO(&list, 2);

    sleep(5);

    fcfs(&list);

    pthread_cancel(schedulerThread);
    pthread_join(schedulerThread, NULL);

    // Free the memory allocated for the nodes
    Node *current = list.head;
    Node *temp;
    while (current != NULL)
    {
        temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}
