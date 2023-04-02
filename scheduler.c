// James Miller
// CST - 315
// 04/02/2023
// Dr. Citro

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef enum
{
    RUN,
    WAIT,
    READY
} State;

typedef struct Process
{
    int processId;
    int burstTime;
    State state;
    struct Process *next;
} Process;

typedef struct ProcessList
{
    Process *head;
    int size;
} ProcessList;

// Function prototypes
void createProcessList(ProcessList *list);
Process *createProcess(int processId, int burstTime);
void addProcess(ProcessList *list, int processId, int burstTime);
void simulateIO(ProcessList *list, int processId, bool ioComplete);
bool isIOCompleted(Process *process);
void *processScheduler(void *arg);
void printProcessQueue(ProcessList *list);

void createProcessList(ProcessList *list)
{
    list->head = NULL;
    list->size = 0;
}

Process *createProcess(int processId, int burstTime)
{
    Process *newProcess = (Process *)malloc(sizeof(Process));
    newProcess->processId = processId;
    newProcess->burstTime = burstTime;
    newProcess->state = READY;
    newProcess->next = NULL;
    return newProcess;
}

void addProcess(ProcessList *list, int processId, int burstTime)
{
    Process *newProcess = createProcess(processId, burstTime);

    if (list->head == NULL)
    {
        list->head = newProcess;
    }
    else
    {
        Process *current = list->head;
        Process *prev = NULL;
        while (current != NULL && current->burstTime < newProcess->burstTime)
        {
            prev = current;
            current = current->next;
        }
        if (prev == NULL)
        {
            newProcess->next = list->head;
            list->head = newProcess;
        }
        else
        {
            newProcess->next = current;
            prev->next = newProcess;
        }
    }
    list->size++;
}

void simulateIO(ProcessList *list, int processId, bool ioComplete)
{
    Process *current = list->head;
    while (current != NULL)
    {
        if (current->processId == processId)
        {
            current->state = ioComplete ? READY : WAIT;
            break;
        }
        current = current->next;
    }
}

bool isIOCompleted(Process *process)
{
    // Simulate I/O completion for demonstration purposes.
    return rand() % 2 == 0;
}

void printProcessQueue(ProcessList *list)
{
    Process *current = list->head;
    printf("Current Process Queue:\n");
    printf("----------------------\n");
    printf("ProcessID  Burst Time  State\n");
    while (current != NULL)
    {
        printf("%-9d  %-10d  ", current->processId, current->burstTime);
        switch (current->state)
        {
        case RUN:
            printf("RUN\n");
            break;
        case WAIT:
            printf("WAIT\n");
            break;
        case READY:
            printf("READY\n");
            break;
        }
        current = current->next;
    }
    printf("\n");
}

void *processScheduler(void *arg)
{
    ProcessList *list = (ProcessList *)arg;
    int waitingTime = 0;
    int turnaroundTime = 0;
    int totalWaitingTime = 0;
    int executedProcesses = 0;

    printf("ProcessID  Waiting Time  Turnaround Time\n");
    printf("----------------------------------------\n");

    while (true)
    {
        printProcessQueue(list);

        Process *current = list->head;
        while (current != NULL)
        {
            if (current->state == READY)
            {
                printf("Executing ProcessID %d\n", current->processId);
                turnaroundTime = waitingTime + current->burstTime;
                printf("%-9d  %-12d  %-15d\n", current->processId, waitingTime, turnaroundTime);

                totalWaitingTime += waitingTime;
                executedProcesses++;
                waitingTime = turnaroundTime;
                current->burstTime = 0;
                current->state = RUN;
            }
            else if (current->state == WAIT && isIOCompleted(current))
            {
                current->state = READY;
            }
            current = current->next;
        }

        if (executedProcesses > 0)
        {
            float avgWaitingTime = (float)totalWaitingTime / executedProcesses;
            printf("Average Waiting Time: %.2f\n", avgWaitingTime);
        }

        sleep(1);
    }

    return NULL;
}

int main()
{
    srand(time(NULL));

    ProcessList list;
    createProcessList(&list);

    addProcess(&list, 1, 5);
    addProcess(&list, 2, 8);
    addProcess(&list, 3, 12);

    pthread_t schedulerThread;
    pthread_create(&schedulerThread, NULL, processScheduler, &list);

    // Simulate I/O request for ProcessID 2
    sleep(2);
    simulateIO(&list, 2, false);

    // Simulate I/O completion for ProcessID 2
    sleep(4);
    simulateIO(&list, 2, true);

    sleep(10);
    pthread_cancel(schedulerThread);
    pthread_join(schedulerThread, NULL);

    // Free the memory allocated for the processes
    Process *current = list.head;
    Process *temp;
    while (current != NULL)
    {
        temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}