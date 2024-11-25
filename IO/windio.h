#pragma once

#define PERM 0666
#define MASZ 1024

#define _IO_MAGIC (1 << 0) // 标志FILE结构体有效
#define _IO_NO_WRITES (1 << 1) // 表示该流不允许写操作
#define _IO_NO_READS (1 << 2) // 表示该流不允许读操作
#define _IOFBF (1 << 3) // 采用全缓冲机制
#define _IOLBF (1 << 4) // 采用行缓冲机制
#define _IONBF (1 << 5) // 采用无缓冲机制

#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

typedef struct _IO_FILE {
	int _fileno; // 文件描述符

	int _flags;  // 描述文件属性

	char* _buf_base; // 缓冲区起始地址
	char* _buf_end; // 缓冲区结尾位置

	char* _read_ptr; // 回退位置
	char* _read_pos; // 第一个有效位置
	char* _read_end; // 最后一个有效字符的下一位置

	char* _write_ptr; // 回退位置
	char* _write_end; // 最后一个有效字符的下一位置

}FILE;

FILE* fopen(const char* path, const char* mode);
int fflush(FILE* stream);
size_t fwrite(const char* ptr, size_t size, size_t nmemb, FILE* stream);
//size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
int fclose(FILE* stream);