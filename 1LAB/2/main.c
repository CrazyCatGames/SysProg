#include "include/2.h"


int main(int argc, char *argv[]) {
	if (argc < 3) {
		HandlePrint(1, "Usage: %s <file1> [file2 ...] <operation>\n", argv[0]);
		return 1;
	}

	char operation[256];
	char *argument = NULL;
	char *last_arg = argv[argc - 1];
	char *bracket_open = strchr(last_arg, '<');
	if (bracket_open) {
		char *bracket_close = strrchr(last_arg, '>');
		if (!bracket_close || bracket_close < bracket_open) {
			HandlePrint(1, "Incorrect format for operation argument\n");
			return 1;
		}

		size_t operation_len = bracket_open - last_arg;
		if (operation_len >= sizeof(operation)) {
			operation_len = sizeof(operation) - 1;
		}

		strncpy(operation, last_arg, operation_len);
		operation[operation_len] = '\0';
		argument = bracket_open + 1;
		*bracket_close = '\0';
	} else {
		strncpy(operation, last_arg, sizeof(operation) - 1);
		operation[sizeof(operation) - 1] = '\0';
	}

	for (int i = 1; i < argc - 1; i++) {
		if (strncmp(operation, "xor", 3) == 0) {
			int n = atoi(operation + 3);
			if (n < 2 || n > 6) {
				HandlePrint(1, "N must be from 2 to 6\n");
				return 1;
			}

			XorN(argv[i], n);
		} else if (strcmp(operation, "mask") == 0) {
			if (!argument) {
				HandlePrint(1, "Missing mask argument\n");
				return 1;
			}

			uint32_t hex_mask;
			if (IsValidHex(argument)) {
				sscanf(argument, "%x", &hex_mask);
				hex_mask = SwapEndian(hex_mask);
			} else {
				HandlePrint(1, "Mask must include only 0-9, a-f, A-F\n");
				return 1;
			}

			Mask(argv[i], hex_mask);
		} else if (strncmp(operation, "copy", 4) == 0) {
			int n = atoi(operation + 4);
			if (n <= 0) {
				HandlePrint(1, "Number of copies must be positive\n");
				continue;
			}

			CopyN(argv[i], n);
		} else if (strcmp(operation, "find") == 0) {
			if (!argument) {
				HandlePrint(1, "Missing find argument\n");
				return 1;
			}

			Parse(argument);
			FindString(argv[i], argument);
		} else {
			HandlePrint(1, "Unknown operation: %s\n", operation);
			return 1;
		}
	}
	return 0;

}
