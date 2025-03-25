#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define INITIAL_USER_CAPACITY 10
#define MAX_USERNAME 7

typedef struct User {
	char username[MAX_USERNAME];
	int pin;
	int sanctions;
} User;

typedef enum OPT {
	AUTH_SUCCESS,
	AUTH_FAIL,
	INCORRECT_DATA,
	ERROR_MEMORY_ALLOC,
	REG_SUCCESS
} OPT;

extern User *users;
extern User *current_user;
extern int user_count;
extern int user_capacity;

extern int sem_id;

void HandlePrint(char code, const char* string, ...);
OPT RegisterUser();
OPT AuthenticateUser();
void* Shell(void *args);
