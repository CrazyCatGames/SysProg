#pragma once

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <unistd.h>

#define PAD_VALUE 0x00

int IsValidHex(const char *str);
void HandlePrint(char code, const char *format, ...);
void XorN(const char *filename, int N);
void Mask(const char *filename, uint32_t hex_mask);
void CopyN(const char *filename, int N);
void FindString(const char *filename, const char *searchStr);
