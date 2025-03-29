#include "../include/2.h"

uint32_t SwapEndian(uint32_t value) {
	return ((value & 0xFF000000) >> 24) |
		   ((value & 0x00FF0000) >> 8) |
		   ((value & 0x0000FF00) << 8) |
		   ((value & 0x000000FF) << 24);
}

void Parse(char *str) {
	char *src = str, *dst = str;
	while (*src) {
		if (src[0] == '\\' && src[1] == 'n') {
			*dst++ = '\n';
			src += 2;
		} else {
			*dst++ = *src++;
		}
	}
	*dst = '\0';
}

int IsValidHex(const char *str) {
	if (!str || *str == '\0') return 0;
	while (*str) {
		if (!isxdigit((unsigned char) *str)) return 0;
		str++;
	}
	return 1;
}

void HandlePrint(char code, const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(code ? stderr : stdout, format, args);
	va_end(args);
}

char SearchInFiles(const char *filename, const char *substring) {
	size_t len_substr = strlen(substring);
	FILE *file = fopen(filename, "r");
	if (!file) {
		perror("Error of opening file\n");
		return -1;
	}

	int c;
	int n_char = 0, n_line = 1;
	int idx = 0;
	int n_line_answ = 0;

	while ((c = getc(file)) != EOF) {
		n_char++;
		if (c == '\n') {
			n_line++;
			n_char = 0;
		}

		if (c == substring[idx]) {
			idx++;
			if (idx == 1) {
				n_line_answ = n_line;
			}

			if (idx == len_substr) {
				return 0;
			}
		} else if (idx > 0) {
			fseek(file, -idx + 1, SEEK_CUR);

			n_char -= (idx - 1);
			if (n_line != n_line_answ) {
				n_line = n_line_answ + 1;
			}
			idx = 0;
		}
	}

	return 1;
}

int CopyFile(const char *src, const char *dest) {
	int in_file = open(src, O_RDONLY);
	if (in_file < 0) {
		perror("Open source file");
		return -1;
	}
	int out_file = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (out_file < 0) {
		perror("Open copied file");
		close(in_file);
		return -1;
	}

	int buffer[4096];
	ssize_t bytes_read;
	while ((bytes_read = read(in_file, buffer, sizeof(buffer))) > 0) {
		ssize_t BytesWritten = write(out_file, buffer, bytes_read);
		if (BytesWritten != bytes_read) {
			perror("Error writing to file");
			close(in_file);
			close(out_file);
			return -1;
		}
	}

	if (bytes_read < 0) {
		perror("Error reading from file");
	}

	close(in_file);
	close(out_file);
	return 0;
}

void XorN(const char *filename, int N) {
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		perror("Error of opening file\n");
		return;
	}

	if (N == 2) {
		unsigned char result = 0;
		int nibble_count = 0;
		int c;
		while ((c = fgetc(fp)) != EOF) {
			unsigned char byte = (unsigned char) c;
			unsigned char high = byte >> 4;
			unsigned char low = byte & 0x0F;

			result ^= high;
			nibble_count++;
			result ^= low;
			nibble_count++;
		}

		if (nibble_count % 2 != 0) {
			result ^= (PAD_VALUE & 0x0F);
		}

		HandlePrint(0, "Xor2 result for %s: %01X\n", filename, result);
		fclose(fp);
		return;
	}

	int block_bits = 1 << N;
	int block_bytes = (block_bits + 7) / 8;

	unsigned char *result = calloc(block_bytes, 1);
	unsigned char *buffer = malloc(block_bytes);
	if (!result || !buffer) {
		perror("Alloc error");
		fclose(fp);
		result ? free(result) : free(buffer);
		return;
	}

	size_t bytes_read;
	while ((bytes_read = fread(buffer, 1, block_bytes, fp)) > 0) {
		if (bytes_read < (size_t) block_bytes) {
			memset(buffer + bytes_read, PAD_VALUE, block_bytes - bytes_read);
		}
		for (int i = 0; i < block_bytes; i++) {
			result[i] ^= buffer[i];
		}
	}

	fclose(fp);
	free(buffer);

	HandlePrint(0, "Xor%d result for %s: ", N, filename);
	for (int i = 0; i < block_bytes; i++) {
		HandlePrint(0, "%02X", result[i]);
	}

	HandlePrint(0, "\n");
	free(result);
}

void Mask(const char *filename, uint32_t hex_mask) {
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		perror("Error of opening file\n");
		return;
	}
	uint32_t number;
	int count = 0;

	while (fread(&number, sizeof(uint32_t), 1, fp) == 1) {
		if ((number & hex_mask) == hex_mask) {
			count++;
		}
	}

	fclose(fp);
	HandlePrint(0, "Mask 0x%X count for %s: %d\n", hex_mask, filename, count);
}

void CopyN(const char *filename, int N) {
	for (int i = 1; i <= N; i++) {
		pid_t pid = fork();
		if (pid < 0) {
			perror("Error while forking\n");
			continue;
		} else if (pid == 0) {
			char dest[PATH_MAX];
			char *dot = strrchr(filename, '.');
			if (dot) {
				snprintf(dest, sizeof(dest), "%.*s_copy%d%s", (int) (dot - filename), filename, i, dot);
			} else {
				snprintf(dest, sizeof(dest), "%s_copy%d", filename, i);
			}

			if (CopyFile(filename, dest) == 0) {
				HandlePrint(0, "Created copy: %s\n", dest);
			} else {
				HandlePrint(1, "Failed to create copy: %s\n", dest);
			}
			exit(0);
		}
	}
	for (int i = 1; i <= N; i++) {
		wait(NULL);
	}
}

void FindString(const char *filename, const char *search_str) {
	pid_t pid = fork();
	if (pid < 0) {
		perror("Error while forking\n");
		return;
	} else if (pid == 0) {
		char code = SearchInFiles(filename, search_str);
		if (code == 0) {
			HandlePrint(0, "String '%s' found in %s\n", search_str, filename);
		} else if (code != -1) {
			HandlePrint(1, "String '%s' not found in %s\n", search_str, filename);
		}
		exit(0);
	} else {
		wait(NULL);
	}
}
