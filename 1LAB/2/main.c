#include "include/2.h"


int main(int argc, char *argv[]) {
	if (argc < 3) {
		HandlePrint(1, "Usage: %s <file1> [file2 ...] <operation>\n", argv[0]);
		return 1;
	}

	char *operation = argv[argc - 1];
	char *argument = NULL;
	size_t len = 0;

	if (strcmp(operation, "mask") == 0 || strcmp(operation, "find") == 0) {
		HandlePrint(0, "Enter argument: ");
		if (getline(&argument, &len, stdin) == -1) {
			perror("getline");
			free(argument);
			return 1;
		}
		size_t arg_len = strlen(argument);
		if (arg_len > 0 && argument[arg_len - 1] == '\n') {
			argument[arg_len - 1] = '\0';
		}
	}

	for (int i = 1; i < argc - 1; i++) {
		if (strncmp(operation, "xor", 3) == 0) {
			int N = atoi(operation + 3);
			if (N < 2 || N > 6) {
				HandlePrint(1, "N must be from 2 to 6\n");
				free(argument);
				return 1;
			}
			XorN(argv[i], N);
		} else if (strcmp(operation, "mask") == 0) {
			if (!IsValidHex(argument)) {
				HandlePrint(1, "Mask must include only 0-9, a-f, A-F\n");
				free(argument);
				return 1;
			}

			uint32_t hex_mask = strtoul(argument, NULL, 16);
			Mask(argv[i], hex_mask);
		} else if (strncmp(operation, "copy", 4) == 0) {
			int N = atoi(operation + 4);
			if (N <= 0) {
				HandlePrint(1, "Number of copies must be positive\n");
				continue;
			}
			CopyN(argv[i], N);
		} else if (strcmp(operation, "find") == 0) {
			FindString(argv[i], argument);
		} else {
			HandlePrint(1, "Unknown operation: %s\n", operation);
			free(argument);
			return 1;
		}
	}

	free(argument);
	return 0;
}
