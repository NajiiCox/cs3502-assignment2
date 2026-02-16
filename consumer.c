#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "buffer.h"
#include <semaphore.h>

static void die(const char* msg)
{
       perror(msg);
       exit(EXIT_FAILURE);
}

int main(int argc, char * argv[])
{
       if(argc != 3)
	{
          fprintf(stderr, "Usage: %s <id> <num_items>\n", argv[0]);
	  return EXIT_FAILURE;
 	}

	int consumer_id = atoi(argv[1]);
	int num_items = atoi(argv[2]);


	int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
	if(shm_id == -1) die("shmget");

	shared_buffer_t* buf = (shared_buffer_t*)shmat(shm_id,NULL, 0);
	if(buf == (void*)-1) die("shmat");

	sem_t* sem_empty = sem_open("/sem_empty", 0);
	if(sem_empty == SEM_FAILED) die ("sem_open empty");

	sem_t* sem_full = sem_open("/sem_full", 0);
	if(sem_full == SEM_FAILED) die ("sem_open full");

	sem_t* sem_mutex = sem_open("/sem_mutex", 0);
	if(sem_mutex == SEM_FAILED) die ("sem_open mutex");

	for(int i = 0; i < num_items; i++)
	{
	 sem_wait(sem_full);
	 sem_wait(sem_mutex);

	 item_t item = buf -> buffer [buf -> tail];
	 buf -> tail = (buf -> tail + 1) % BUFFER_SIZE;
	 buf -> count--;

	 sem_post(sem_mutex);
	 sem_post (sem_empty);

	 printf("Consumer %d: Consumed value %d from Producer %d\n", consumer_id, item.value, item.producer_id);
	 fflush(stdout);
}

shmdt(buf);
sem_close(sem_empty);
sem_close(sem_full);
sem_close(sem_mutex);

return EXIT_SUCCESS;
}
