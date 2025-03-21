#include "../include/1.h"

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
	if (!lineptr || !n || !stream) return -1;

	int ch;
	size_t pos = 0;

	if (*lineptr == NULL) {
		*n = 128;
		*lineptr = malloc(*n);
		if (!*lineptr) return -1;
	}

	while ((ch = fgetc(stream)) != EOF && ch != '\n') {
		if (pos + 1 >= *n) {// Увеличение буфера при необходимости
			*n *= 2;
			char *new_ptr = realloc(*lineptr, *n);
			if (!new_ptr) return -1;
			*lineptr = new_ptr;
		}
		(*lineptr)[pos++] = ch;
	}

	if (pos == 0 && ch == EOF) return -1;// Если сразу EOF, вернуть -1

	(*lineptr)[pos] = '\0';// Завершаем строку
	return pos;
}

void FreeUsers() {
	free(users);
}

void GetValidData(char *username, int *pin) {
	char input[10];
	char pin_input[10];

	while (1) {
		printf("Enter login (max 6 symbols, latin letters and numbers): ");
		scanf("%9s", input);

		if (strlen(input) > MAX_USERNAME) {
			printf("Error: length of username mustn't be more than 6.\n");
			continue;
		}

		int valid = 1;
		for (int i = 0; input[i] != '\0'; i++) {
			if (!isalnum((unsigned char) input[i])) {
				valid = 0;
				break;
			}
		}

		if (!valid) {
			printf("Error: Login must has only latin letters and numbers.\n");
			continue;
		}

		strcpy(username, input);
		break;
	}

	while (1) {
		printf("Enter PIN-code (0-100000): ");
		scanf("%9s", pin_input);
		while (getchar() != '\n')
			;

		int valid = 1;
		for (int i = 0; pin_input[i] != '\0'; i++) {
			if (!isdigit((unsigned char) pin_input[i])) {
				valid = 0;
				break;
			}
		}

		if (!valid) {
			printf("Error: PIN-code must has only numbers.\n");
			continue;
		}

		char *end;
		*pin = (int)strtod(pin_input, &end);

		if (*pin <= 0 || *pin >= 100000) {
			printf("Error: PIN-code must be from 0 to 100000.\n");
			continue;
		}

		break;
	}
}

OPT RegisterUser() {
	if (user_count >= user_capacity) {
		user_capacity *= 2;
		User *new_users = realloc(users, user_capacity * sizeof(User));
		if (!new_users) {
			return ERROR_MEMORY_ALLOC;
		}
		users = new_users;
	}

	GetValidData(users[user_count].username, &users[user_count].pin);
	users[user_count].sanctions = -1;

	printf("User %s registered successful!\n", users[user_count++].username);
	return REG_SUCCESS;
}

OPT AuthenticateUser() {
	char username[MAX_USERNAME + 1];
	int pin;

	GetValidData(username, &pin);

	for (int i = 0; i < user_count; i++) {
		if (strcmp(users[i].username, username) == 0 && users[i].pin == pin) {
			current_user = &users[i];
			return AUTH_SUCCESS;
		}
	}

	printf("Incorrect login or PIN.\n");
	return AUTH_FAIL;
}

void *Shell(void *args) {
	char *command = NULL;
	size_t buffer_size = 0;
	int current_commands = 0;

	printf("Добро пожаловать, %s!\n", current_user->username);

	while (1) {
		if (current_user->sanctions != -1 && current_commands >= current_user->sanctions) {
			printf("Request limit reached. Logging out.\n");
			break;
		}

		printf("> ");
		ssize_t len = getline(&command, &buffer_size, stdin);

		if (len == -1) {
			printf("Error of reading command.\n");
			continue;
		}

		if (command[len - 1] == '\n') {
			command[len - 1] = '\0';
		}

		if (strcmp(command, "Logout") == 0) {
			printf("Logout...\n");
			break;
		}

		if (strcmp(command, "Time") == 0) {
			time_t now = time(NULL);
			struct tm *tm_info = localtime(&now);
			printf("Текущее время: %02d:%02d:%02d\n",
				   tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
			current_commands++;
			continue;
		}

		if (strcmp(command, "Date") == 0) {
			time_t now = time(NULL);
			struct tm *tm_info = localtime(&now);
			printf("Current date: %02d-%02d-%04d\n",
				   tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
			current_commands++;
			continue;
		}

		if (strncmp(command, "Howmuch ", 8) == 0) {
			int year, month, day;
			char flag[4];

			if (sscanf(command + 8, "%2d-%2d-%4d %4s", &day, &month, &year, flag) != 4) {
				printf("Error: command format 'Howmuch DD-MM-YYYY -s/-m/-h/-y'\n");
				continue;
			}

			if (strlen(flag) != 2) {
				printf("Unknown flag %s\n", flag);
				continue;
			}

			if (month < 1 || month > 12 || day < 1 || day > 31) {
				printf("Error: incorrect date.\n");
				continue;
			}

			struct tm input_date = {0};
			input_date.tm_year = year - 1900;
			input_date.tm_mon = month - 1;
			input_date.tm_mday = day;
			time_t input_time = mktime(&input_date);

			if (input_time == -1) {
				printf("Error: impossible to check date.\n");
				continue;
			}

			time_t now = time(NULL);
			double diff = difftime(now, input_time);

			if (diff < 0) {
				printf("Error: input date in the future.\n");
				continue;
			}

			if (strcmp(flag, "-s") == 0) {
				printf("Difference: %.0f seconds\n", diff);
			} else if (strcmp(flag, "-m") == 0) {
				printf("Difference: %.2f minutes\n", diff / 60);
			} else if (strcmp(flag, "-h") == 0) {
				printf("Difference: %.2f hours\n", diff / 3600);
			} else if (strcmp(flag, "-y") == 0) {
				printf("Difference: %.2f years\n", diff / 31536000);
			} else {
				printf("Error: unknown flag '%s'. Use -s/-m/-h/-y.\n", flag);
			}
			current_commands++;
			continue;
		}

		if (strncmp(command, "Sanctions ", 10) == 0) {
			char username[MAX_USERNAME + 1];
			int limit, code;

			if (sscanf(command + 10, "%6s %d", username, &limit) != 2) {
				printf("Error: use 'Sanctions <username> <number>'.\n");
				continue;
			}

			printf("Error confirm code: ");
			if (scanf("%d", &code) != 1 || code != 12345) {
				printf("Error: incorrect code.\n");
				while (getchar() != '\n')
					;
				continue;
			}
			while (getchar() != '\n')
				;

			if (sscanf(command + 10, "%6s %d", username, &limit) == 2) {
				int found = 0;
				for (int i = 0; i < user_count; i++) {
					if (strcmp(users[i].username, username) == 0) {
						users[i].sanctions = limit;
						printf("Set limit of commands for %s: %d\n", username, limit);
						found = 1;
						break;
					}
				}
				if (!found) {
					printf("Error: user '%s' not found.\n", username);
				}
			} else {
				printf("Error: use format 'Sanctions <username> <number>'.\n");
			}
			current_commands++;
			continue;
		}

		printf("Error: unknown command '%s'.\n", command);
	}

	free(command);
	return NULL;
}
