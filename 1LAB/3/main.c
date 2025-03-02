#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>

#define NUM_PHILS 5
#define LEFT ((phil_id + 4) % NUM_PHILS)
#define RIGHT ((phil_id + 1) % NUM_PHILS)
#define THINKING 2
#define HUNGRY 1
#define EATING 0

int state[NUM_PHILS];
int times = 0;
int sem_id;

void* Philosopher(void* phil_id);
void CheckForks(int phil_id);
void TakeFork(int phil_id);
void PutFork(int phil_id);

int main() {
	int i;
	pthread_t thread_id[NUM_PHILS];
	int phils[NUM_PHILS] = {0, 1, 2, 3, 4};

	sem_id = semget(IPC_PRIVATE, NUM_PHILS + 1, IPC_CREAT | 0666);
	if (sem_id == -1) {
		fprintf(stderr, "Error of creating sem");
		return 1;
	}

	semctl(sem_id, NUM_PHILS, SETVAL, 1);
	for (i = 0; i < NUM_PHILS; i++) {
		semctl(sem_id, i, SETVAL, 0);
	}

	for (i = 0; i < NUM_PHILS; i++) {
		pthread_create(&thread_id[i], NULL, Philosopher, &phils[i]);
		printf("Philosopher %d is thinking\n", i + 1);
	}

	for (i = 0; i < NUM_PHILS; i++) {
		pthread_join(thread_id[i], NULL);
	}

	semctl(sem_id, 0, IPC_RMID);
	return 0;
}

void* Philosopher(void* phil_id) {
	int* i = (int*)phil_id;
	while (times < 50) {
		usleep(100000);
		TakeFork(*i);
		usleep(100000);
		PutFork(*i);
		times++;
	}
	return NULL;
}

void TakeFork(int phil_id) {
	struct sembuf op_dec = {NUM_PHILS, -1, 0};
	semop(sem_id, &op_dec, 1);

	state[phil_id] = HUNGRY;
	printf("Philosopher %d is Hungry\n", phil_id + 1);
	CheckForks(phil_id);

	struct sembuf op_inc = {NUM_PHILS, 1, 0};
	semop(sem_id, &op_inc, 1);

	struct sembuf op_wait = {phil_id, -1, 0};
	semop(sem_id, &op_wait, 1);
	usleep(50000);
}

void PutFork(int phil_id) {
	struct sembuf op_dec = {NUM_PHILS, -1, 0};
	semop(sem_id, &op_dec, 1);

	state[phil_id] = THINKING;
	printf("Philosopher %d is putting forks %d and %d down and thinking now\n", phil_id + 1, LEFT + 1, phil_id + 1);
	CheckForks(LEFT);
	CheckForks(RIGHT);

	struct sembuf op_inc = {NUM_PHILS, 1, 0};
	semop(sem_id, &op_inc, 1);
}

void CheckForks(int phil_id) {
	if (state[phil_id] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
		state[phil_id] = EATING;
		printf("Philosopher %d is taking forks %d and %d and eating now\n", phil_id + 1, LEFT + 1, phil_id + 1);

		struct sembuf op_post = {phil_id, 1, 0};
		semop(sem_id, &op_post, 1);
	}
}