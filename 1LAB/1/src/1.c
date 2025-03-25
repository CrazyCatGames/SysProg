#include "../include/1.h"


void HandlePrint(char code, const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(code ? stderr : stdout, format, args);
	va_end(args);
}

void GetValidData(char *username, int *pin) {
	char input[10];
	char pin_input[10];

	while (1) {
		HandlePrint(0, "Enter login (max 6 symbols, latin letters and numbers): ");

		if (scanf("%9s", input) != 1) {
			HandlePrint(0, "Error: input failed.\n");
			while (getchar() != '\n')
				;
			continue;
		}
		while (getchar() != '\n')
			;

		if (strlen(input) > MAX_USERNAME) {
			HandlePrint(0, "Error: length of username mustn't be more than 6.\n");
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
			HandlePrint(0, "Error: Login must contain only latin letters and numbers.\n");
			continue;
		}

		strcpy(username, input);
		break;
	}

	while (1) {
		HandlePrint(0, "Enter PIN-code (0-100000): ");

		if (scanf("%9s", pin_input) != 1) {
			HandlePrint(0, "Error: input failed.\n");
			while (getchar() != '\n')
				;
			continue;
		}
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
			HandlePrint(0, "Error: PIN-code must contain only numbers.\n");
			continue;
		}

		char *end;
		*pin = (int) strtol(pin_input, &end, 10);

		if (*pin <= 0 || *pin >= 100000) {
			HandlePrint(0, "Error: PIN-code must be from 0 to 100000.\n");
			continue;
		}

		break;
	}
}

OPT RegisterUser() {
	char temp_username[MAX_USERNAME];
	int temp_pin;

	GetValidData(temp_username, &temp_pin);

	for (int i = 0; i < user_count; i++) {
		if (strcmp(users[i].username, temp_username) == 0) {
			HandlePrint(0, "User with login '%s' already exists.\n", temp_username);
			return INCORRECT_DATA;
		}
	}

	if (user_count >= user_capacity) {
		user_capacity *= 2;
		User *new_users = (User *) realloc(users, user_capacity * sizeof(User));
		if (!new_users) {
			return ERROR_MEMORY_ALLOC;
		}
		users = new_users;
	}

	strcpy(users[user_count].username, temp_username);
	users[user_count].pin = temp_pin;
	users[user_count].sanctions = -1;

	HandlePrint(0, "User %s registered successful!\n", users[user_count++].username);
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

	return AUTH_FAIL;
}

void *Shell(void *args) {
	char *command = NULL;
	size_t buffer_size = 0;
	int current_commands = 0;

	HandlePrint(0, "Welcome, %s!\n", current_user->username);

	while (1) {
		if (current_user->sanctions != -1 && current_commands >= current_user->sanctions) {
			HandlePrint(0, "Request limit reached. Logging out.\n");
			break;
		}

		HandlePrint(0, "> ");
		ssize_t len = getline(&command, &buffer_size, stdin);

		if (len == -1) {
			HandlePrint(1, "Error of reading command.\n");
			continue;
		}

		if (command[len] == '\n') {
			command[len] = '\0';
		}

		if (strcmp(command, "Logout") == 0) {
			HandlePrint(0, "Logout...\n");
			break;
		}

		if (strcmp(command, "Time") == 0) {
			time_t now = time(NULL);
			struct tm *tm_info = localtime(&now);
			HandlePrint(0, "Current time: %02d:%02d:%02d\n",
						tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
			current_commands++;
			continue;
		}

		if (strcmp(command, "Date") == 0) {
			time_t now = time(NULL);
			struct tm *tm_info = localtime(&now);
			HandlePrint(0, "Current date: %02d-%02d-%04d\n",
						tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
			current_commands++;
			continue;
		}

		if (strncmp(command, "Howmuch ", 8) == 0) {
			int year, month, day;
			char flag[4], year_str[6];

			if (sscanf(command + 8, "%2d-%2d-%s %3s\n", &day, &month, year_str, flag) != 4 || strlen(year_str) > 4) {
				HandlePrint(0, "Error: command format 'Howmuch DD-MM-YYYY -s/-m/-h/-y'\n");
				continue;
			}

			year = atoi(year_str);

			if (strlen(flag) != 2) {
				HandlePrint(0, "Unknown flag %s\n", flag);
				continue;
			}

			if (month < 1 || month > 12 || day < 1 || day > 31) {
				HandlePrint(0, "Error: incorrect date.\n");
				continue;
			}

			struct tm input_date = {0};
			input_date.tm_year = year - 1900;
			input_date.tm_mon = month - 1;
			input_date.tm_mday = day;
			time_t input_time = mktime(&input_date);

			if (input_time == -1) {
				HandlePrint(0, "Error: impossible to check date.\n");
				continue;
			}

			time_t now = time(NULL);
			double diff = difftime(now, input_time);

			if (diff < 0) {
				HandlePrint(0, "Error: input date in the future.\n");
				continue;
			}

			if (strcmp(flag, "-s") == 0) {
				HandlePrint(0, "Difference: %.0f seconds\n", diff);
			} else if (strcmp(flag, "-m") == 0) {
				HandlePrint(0, "Difference: %.2f minutes\n", diff / 60);
			} else if (strcmp(flag, "-h") == 0) {
				HandlePrint(0, "Difference: %.2f hours\n", diff / 3600);
			} else if (strcmp(flag, "-y") == 0) {
				HandlePrint(0, "Difference: %.2f years\n", diff / 31536000);
			} else {
				HandlePrint(0, "Error: unknown flag '%s'. Use -s/-m/-h/-y.\n", flag);
			}
			current_commands++;
			continue;
		}

		if (strncmp(command, "Sanctions ", 10) == 0) {
			char username[MAX_USERNAME + 1];
			int limit, code;

			if (sscanf(command + 10, "%6s %d", username, &limit) != 2 || limit < 0 || limit > 1000) {
				HandlePrint(0, "Error: use 'Sanctions <username> <number (1-1000)>'.\n");
				continue;
			}

			HandlePrint(0, "Enter confirm code: ");
			if (scanf("%d", &code) != 1 || code != 12345) {
				HandlePrint(0, "Error: incorrect code.\n");
				while (getchar() != '\n')
					;
				continue;
			}
			while (getchar() != '\n')
				;

			struct sembuf sem_lock = {0, -1, 0};
			struct sembuf sem_unlock = {0, 1, 0};

			if (semop(sem_id, &sem_lock, 1) == -1) {
				perror("semop lock failed");
			}

			int found = 0;
			for (int i = 0; i < user_count; i++) {
				if (strcmp(users[i].username, username) == 0) {
					users[i].sanctions = limit;
					HandlePrint(0, "Set limit of commands for %s: %d\n", username, limit);
					found = 1;
					break;
				}
			}
			if (!found) {
				HandlePrint(0, "Error: user '%s' not found.\n", username);
			}

			if (semop(sem_id, &sem_unlock, 1) == -1) {
				perror("semop unlock failed");
			}
			current_commands++;
			continue;
		}

		HandlePrint(0, "Error: unknown command '%s'.\n", command);
	}

	free(command);
	return NULL;
}
