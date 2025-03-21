#include "include/1.h"

User *current_user = NULL;
User *users = NULL;
int user_count = 0;
int user_capacity = INITIAL_USER_CAPACITY;

int main() {
	system("chcp 65001");
	int choice = 0;
	pthread_t shell_thread;
	users = malloc(user_capacity * sizeof(User));
	if (!users) {
		fprintf(stderr, "Alloc memory errror.");
		return 1;
	}

	while (1) {
		if (choice == -1) break;

		printf("\n1. Registration\n2. Login\n3. Exit\nChoice: ");

		if (scanf("%d", &choice) != 1) {
			printf("Error: Choose correct number.\n");
			while (getchar() != '\n')
				;
			continue;
		}

		switch (choice) {
			case 1:
				if (RegisterUser() != REG_SUCCESS) {
					fprintf(stderr, "Alloc memory error.\n");
					FreeUsers();
					break;
				}
				break;
			case 2:
				if (!user_count){
					printf("No users exist.\n");
					break;
				}
				if (AuthenticateUser() == AUTH_SUCCESS) {
					printf("Login successful.\n");
					pthread_create(&shell_thread, NULL, Shell, NULL);
					pthread_join(shell_thread, NULL);
					current_user = NULL;
				}else {
					printf("Login failed.\n");
				}
				break;
			case 3:
				printf("Exiting...\n");
				FreeUsers();
				choice = -1;
				break;
			default:
				printf("Incorrect choice.\n");
		}
	}
	return 0;
}
