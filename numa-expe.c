#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define ALLOCATION_SIZE 200000000 // 200 Mega bytes
#define ALLOCATION_CORE 0

int read_core = 0;

// Set CPU affinity for current thread
void setCPUAffinity(int core) {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(core, &cpu_set);
    if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set) < 0) {
	printf("Error setting pthread affinity\n");
	exit(-1);
    }
}

void *alloc(void *param) {
    setCPUAffinity(ALLOCATION_CORE);
    int *alloc_start;
    void *result = malloc(ALLOCATION_SIZE * sizeof(int));
    printf("Allocation done\n");
    return result;
}

void *readWrite(void *param) {
    setCPUAffinity(read_core);
    int *alloc_start = (int *)param;
    int a, i, j;
    for (j = 0; j < 10; j++) {
	for (i = 0; i < ALLOCATION_SIZE; i++) {
	    a = *(alloc_start + i);
	}
    }
    printf("Read thread ended\n");
    return (void *)a;
}

int main() {

    // Runs allocation's thread on ALLOCATION_CORE
    pthread_t alloc_thread;
    if (pthread_create(&alloc_thread, NULL, alloc, NULL)) {
	printf("Failed to create allocation's thread\n");
	return -1;
    }
    int *alloc_start;
    if (pthread_join(alloc_thread, (void **)(&alloc_start))) {
	printf("Failed to join allocation's thread\n");
	return -1;
    }

    // Write integers in allocated memory
    int i;
    for (i = 0; i < ALLOCATION_SIZE; i++) {
	*(alloc_start + i) = i;
    }

    // Runs read's thread
    pthread_t read_thread;
    if (pthread_create(&read_thread, NULL, readWrite, alloc_start)) {
	printf("Failed to create read's thread\n");
	return -1;
    }

    // Joins read thread
    if (pthread_join(alloc_thread, NULL)) {
	printf("Failed to join read's thread\n");
	return -1;
    }

    return 0;
}
