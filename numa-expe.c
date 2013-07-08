#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <numa.h>
#include <stdint.h>
#include <errno.h>

#define ALLOCATION_SIZE 20000000 // 20 mega bytes * size of int
#define ALLOCATION_CORE 0
#define NB_CORES 12
#define NB_ITERATIONS 20

char global_data_1[4096];
char global_data_2[4096];
void *global_data;
int global_data_core;
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

// Check global data allocation location
void *checkGlobalAlloc(void *param) {
    setCPUAffinity(global_data_core);
    *((int *)global_data) = 12;
    uintptr_t ptr = (uintptr_t)global_data;
    if (ptr % 4096 != 0) {
    	ptr = ptr - ptr % 4096;
    }
    int status[0];
    if (move_pages(0 /*self memory */, 1, &ptr, NULL, status, 0)) {
    	printf("Move pages error %s\n", strerror(errno));
    	exit(-1);
    }
    printf("Memory at %p first touched on core %d is on %d node\n", ptr, global_data_core, status[0]);
}

// Allocate and touch every memory pages
void *allocAndTouch(void *param) {
    setCPUAffinity(ALLOCATION_CORE);
    void *result = malloc(ALLOCATION_SIZE * sizeof(int));
    if (result == NULL) {
	printf("Memory allocation (size = %d) failed\n", ALLOCATION_SIZE * sizeof(int));
	exit(-1);
    }
    int i;
    for (i = 0; i < ALLOCATION_SIZE; i++) {
	*(int *)(result + i) = i;
    }
    printf("Allocation of %d bytes done\n", ALLOCATION_SIZE * sizeof(int));
    return result;
}

void *readWrite(void *param) {
    setCPUAffinity(read_core);
    int *alloc_start = (int *)param;
    int a, i, j;
    struct timeval t1, t2;
    double elapsedTime;
    gettimeofday(&t1, NULL);
    for (j = 0; j < NB_ITERATIONS; j++) {
	for (i = 0; i < ALLOCATION_SIZE; i++) {
	    a = *(alloc_start + (i * 1009l) %ALLOCATION_SIZE);
	}
    }
    gettimeofday(&t2, NULL);
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    printf("Read and write from core %2d on core %2d ==> %0.3f\n", read_core, ALLOCATION_CORE, elapsedTime);
    return NULL;
}

int main() {

    // Check we are on a NUMA system
    if (numa_available() == -1) {
	printf("System is not NUMA, exiting !\n");
	return -1;
    }
    printf("The NUMA world is %d big\n", numa_num_configured_nodes());

    // Check where global data is allocated
    /* pthread_t check_alloc_thread; */
    /* global_data = global_data_1; */
    /* global_data_core = 6; */
    /* if (pthread_create(&check_alloc_thread, NULL, checkGlobalAlloc, NULL)) { */
    /* 	printf("Failed to create check allocation's thread\n"); */
    /* 	return -1; */
    /* } */
    /* if (pthread_join(check_alloc_thread, NULL)) { */
    /* 	printf("Failed to join check allocation's thread\n"); */
    /* 	return -1; */
    /* } */
    /* global_data = global_data_2; */
    /* global_data_core = 0; */
    /* if (pthread_create(&check_alloc_thread, NULL, checkGlobalAlloc, NULL)) { */
    /* 	printf("Failed to create check allocation's thread\n"); */
    /* 	return -1; */
    /* } */
    /* if (pthread_join(check_alloc_thread, NULL)) { */
    /* 	printf("Failed to join check allocation's thread\n"); */
    /* 	return -1; */
    /* } */


    // Runs allocation's thread on ALLOCATION_CORE
    pthread_t alloc_thread;
    if (pthread_create(&alloc_thread, NULL, allocAndTouch, NULL)) {
	printf("Failed to create allocation's thread\n");
	return -1;
    }
    int *alloc_start;
    if (pthread_join(alloc_thread, (void **)(&alloc_start))) {
	printf("Failed to join allocation's thread\n");
	return -1;
    }


    // Iterate over cores
    for (read_core = 0; read_core < NB_CORES; read_core++) {

	// Runs read's and write thread
	pthread_t read_thread;
	if (pthread_create(&read_thread, NULL, readWrite, alloc_start)) {
	    printf("Failed to create read's thread\n");
	    return -1;
	}

	// Joins read and write thread
	if (pthread_join(alloc_thread, NULL)) {
	    printf("Failed to join read's thread\n");
	    return -1;
	}
    }

    return 0;
}
