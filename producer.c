#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include "buffer.h"

static void die(const char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]){
	if(argc != 3){
	   fprintf(stderr, "Usage: %s <id> <num_items>\n", argv[0]);
	   return EXIT_FAILURE;
}


	int producer_id = atoi(argv[1]);
	int num_items = atoi(argv[2]);

	int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);
	if(shm_id == -1) die("shmget");

	shared_buffer_t* buf = (shared_buffer_t*)shmat(shm_id, NULL, 0);
	if(buf == (void*)-1)die("shmat");

	sem_t* sem_empty = sem_open("/sem_empty", O_CREAT, 0644, BUFFER_SIZE);
	if(sem_empty == SEM_FAILED) die("sem_open empty");

	sem_t* sem_full = sem_open("/sem_full", O_CREAT, 0644, 0);
	if(sem_full == SEM_FAILED) die("sem_open full");

	sem_t* sem_mutex = sem_open("/sem_mutex", O_CREAT, 0644,1);
	if(sem_mutex == SEM_FAILED) die("sum_open mutex");

	if(buf->count < 0 || buf->count > BUFFER_SIZE){
	buf->head = 0;
	buf->tail = 0;
	buf->count = 0;
}
	for(int i =0; i < num_items; i++){
	    int value = producer_id * 1000 + i;

	    sem_wait(sem_empty);
	    sem_wait(sem_mutex);


	    buf->buffer[buf->head].value = value;
	    buf->buffer[buf->head].producer_id = producer_id;

	    buf->head = (buf->head + 1) % BUFFER_SIZE;
	    buf->count++;

	    sem_post(sem_mutex);
	    sem_post(sem_full);

	    printf("Producer %d: Produced value %d\n", producer_id, value);
	    fflush(stdout);
}

	shmdt(buf);
	sem_close(sem_empty);
	sem_close(sem_full);
	sem_close(sem_mutex);

	return EXIT_SUCCESS;
}



