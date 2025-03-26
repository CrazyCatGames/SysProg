#include "../include/2.h"


void HandlePrint(char code, const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(code ? stderr : stdout, format, args);
	va_end(args);
}

static int CopyFile(const char *src, const char *dest) {
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

	struct stat st;
	size_t buf_size = 4096;
	if (fstat(in_file, &st) == 0 && st.st_size > 0) {
		buf_size = st.st_size;
	}

	char *buffer = (char *) malloc(buf_size * sizeof(char));
	if (!buffer) {
		perror("Malloc");
		close(in_file);
		close(out_file);
		return -1;
	}

	ssize_t n;
	while ((n = read(in_file, buffer, buf_size)) > 0) {
		if (write(out_file, buffer, n) != n) {
			perror("Write");
			close(in_file);
			close(out_file);
			free(buffer);
			return -1;
		}
	}
	close(in_file);
	close(out_file);
	free(buffer);
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
			unsigned char byte = (unsigned char)c;
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
	unsigned char buf[4];

	while (fread(buf, 1, 4, fp) == 4) {
		number = ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) |
				 ((uint32_t) buf[2] << 8) | ((uint32_t) buf[3]);
		if ((number & hex_mask) == hex_mask) {
			count++;
		}
	}

	int remain = 0;
	while (remain < 4) {
		int c = fgetc(fp);
		if (c == EOF) break;
		buf[remain++] = (unsigned char) c;
	}

	if (remain > 0 && remain < 4) {
		memset(buf + remain, PAD_VALUE, 4 - remain);
		number = ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) |
				 ((uint32_t) buf[2] << 8) | ((uint32_t) buf[3]);
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
			char dest[512];
			snprintf(dest, sizeof(dest), "%s_copy%d", filename, i);
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
		FILE *fp = fopen(filename, "r");
		if (!fp) {
			perror("Error of opening file\n");
			exit(1);
		}

		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		int found = 0;
		while ((read = getline(&line, &len, fp)) != -1) {
			if (strstr(line, search_str) != NULL) {
				found = 1;
				break;
			}
		}
		free(line);
		fclose(fp);

		if (found) {
			HandlePrint(0, "String '%s' found in %s\n", search_str, filename);
		} else {
			HandlePrint(1, "String '%s' not found in %s\n", search_str, filename);
		}
		exit(0);
	} else {
		wait(NULL);
	}
}
