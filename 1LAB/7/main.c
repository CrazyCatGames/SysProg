#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

char FileType(mode_t mode) {
	if (S_ISDIR(mode)) return 'd';  //Dir
	if (S_ISREG(mode)) return '-';  //File
	if (S_ISLNK(mode)) return 'l';  //Symbol link
	if (S_ISCHR(mode)) return 'c';  //Symbol device
	if (S_ISBLK(mode)) return 'b';  //Block device
	if (S_ISFIFO(mode)) return 'p'; //Named pipe
	if (S_ISSOCK(mode)) return 's'; //Socket
	return '?';
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Use: %s <directory> [<directory> ...]\n", argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		char *dir_name = argv[i];
		DIR *dir = opendir(dir_name);
		if (!dir) {
			fprintf(stderr, "Error of opening directory %s\n", dir_name);
			continue;
		}
		printf("Directory: %s\n", dir_name);

		struct dirent *current_dir;
		while ((current_dir = readdir(dir))) {
			if (strcmp(current_dir->d_name, ".") == 0 || strcmp(current_dir->d_name, "..") == 0 /* || current_dir->d_name[0] == '.'*/ ) continue;

			char path[PATH_MAX];
			snprintf(path, sizeof(path), "%s/%s", dir_name, current_dir->d_name);

			struct stat st;
			if (lstat(path, &st) == -1) {
				perror("lstat\n");
				continue;
			}
			printf("%c %lu %s\n", FileType(st.st_mode), st.st_ino, current_dir->d_name);
		}
		closedir(dir);
		printf("\n");
	}

	return 0;
}
