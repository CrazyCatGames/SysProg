#include <Windows.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define NUM_PHILS 5
#define LEFT ((phil_id + 4) % NUM_PHILS)
#define RIGHT ((phil_id + 1) % NUM_PHILS)
#define THINKING 2
#define HUNGRY 1
#define EATING 0

int state[NUM_PHILS];
int times = 0;
sem_t mutex;
sem_t phile_thread[NUM_PHILS];

void* Philosopher(void* phil_id);
void CheckForks(int phil_id);
void TakeFork(int phil_id);
void PutFork(int phil_id);

int main() {
	int i;
	pthread_t thread_id[NUM_PHILS];
	int phils[NUM_PHILS] = {0, 1, 2, 3, 4};

	sem_init(&mutex, 0, 1);

	for (i = 0; i < NUM_PHILS; i++) {
		sem_init(&phile_thread[i], 0, 0);
	}

	for (i = 0; i < NUM_PHILS; i++) {
		pthread_create(&thread_id[i], NULL, Philosopher, &phils[i]);
		printf("Philosopher %d is thinking\n", i + 1);
	}

	printf("\n");
	for (i = 0; i < NUM_PHILS; i++) {
		pthread_join(thread_id[i], NULL);
	}
}

void* Philosopher(void* phil_id) {
	while (times < 50) {
		int* i = phil_id;
		Sleep(100);
		TakeFork(*i);
		Sleep(100);
		PutFork(*i);
		times++;
	}
}

void TakeFork(int phil_id) {
	sem_wait(&mutex);

	state[phil_id] = HUNGRY;
	printf("Philosopher %d is Hungry\n", phil_id + 1);
	CheckForks(phil_id);

	sem_post(&mutex);
	sem_wait(&phile_thread[phil_id]);

	Sleep(50);
}

void PutFork(int phil_id) {
	sem_wait(&mutex);

	state[phil_id] = THINKING;
	printf("Philosopher %d is putting forks %d and %d down and thinking now\n", phil_id + 1, LEFT + 1, phil_id + 1);
	CheckForks(LEFT);
	CheckForks(RIGHT);

	sem_post(&mutex);
}

void CheckForks(int phil_id) {
	if (state[phil_id] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
		state[phil_id] = EATING;

		printf("Philosopher %d takes forks %d and %d\n", phil_id + 1, LEFT + 1, phil_id + 1);
		printf("Philosopher %d is Eating\n", phil_id + 1);

		sem_post(&phile_thread[phil_id]);
	}
}
