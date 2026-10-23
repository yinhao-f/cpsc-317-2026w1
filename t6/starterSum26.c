#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Arrays for storing thread and task IDs
pthread_t threads[50];
int task_ids[50];

// A function that simulates a task
void* runTask(void* task_id_ptr) {
    int task_id = *(int*)task_id_ptr;

    // Sleep is used to simulate work
    // Each task takes 0.1 - 0.2 seconds
    int sleepTime = 100000 + rand() % 100000; 
    usleep(sleepTime);

    printf("Completed task: %d\n", task_id);
    return NULL;
}

int main() {
    srand(time(NULL));

    // Initialization for task IDs
    for (int i = 0; i < 50; i++) {
        task_ids[i] = i;
    }
    
    printf("\n----- Part 1 -----\n");

    // Part 1: Use threads to complete 50 tasks
    // TODO - Your code here
    for (int i = 0; i < 50; i++) {
	pthread_create(&threads[i], NULL, &runTask, &task_ids[i]);
    }


    // Joining all threads
    for (int i = 0; i < 50; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n----- Part 2 -----\n");

    // Part 2: Use threads to complete 50 tasks, but with the following condition:
    //          - Run the tasks in groups of 10
    //          - A group must finish before the next group starts
    // TODO - Your code here

    return 0;
}
