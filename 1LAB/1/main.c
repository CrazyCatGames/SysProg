#include "include/1.h"

User *current_user = NULL;
User *users = NULL;
int user_count = 0;
int user_capacity = INITIAL_USER_CAPACITY;

int sem_id = 0;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

int main() {
	int choice = 0;
	pthread_t shell_thread;
	users = malloc(user_capacity * sizeof(User));
	if (!users) {
		HandlePrint(1,"Alloc memory errror.");
		return 1;
	}

	sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if (sem_id == -1) {
		HandlePrint(1, "Failed to create semaphore.");
		return 1;
	}
	union semun sem_union;
	sem_union.val = 1;
	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
		HandlePrint(1, "Failed to initialize semaphore.");
		return 1;
	}

	while (1) {
		if (choice == -1) break;

		HandlePrint(0, "1. Registration\n2. Login\n3. Exit\nChoice: ");

		if (scanf("%d", &choice) != 1) {
			HandlePrint(0, "Error: Choose correct number.\n");
			continue;
		}

		switch (choice) {
			case 1:
				OPT tmp = RegisterUser();
				if (tmp == ERROR_MEMORY_ALLOC) {
					HandlePrint(1,"Alloc memory error.\n");
					FreeUsers();
					choice = -1;
				}
				break;
			case 2:
				if (!user_count){
					HandlePrint(0, "No users exist.\n");
					break;
				}
				if (AuthenticateUser() == AUTH_SUCCESS) {
					HandlePrint(0, "Login successful.\n");
					pthread_create(&shell_thread, NULL, Shell, NULL);
					pthread_join(shell_thread, NULL);
					current_user = NULL;
				}else {
					HandlePrint(0, "Login failed. Incorrect login or PIN.\n");
				}
				break;
			case 3:
				HandlePrint(0, "Exiting...\n");
				FreeUsers();
				choice = -1;
				break;
			default:
				HandlePrint(0, "Incorrect choice.\n");
		}
	}
	return 0;
}
