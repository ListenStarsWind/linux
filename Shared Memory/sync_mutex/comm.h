#pragma once

#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<string>


#define FIFO_NAME "myfifo" // The name of the pipe
#define MODE 0664          // The creation mode of the pipe

enum exit_code
{
    FIFO_CREATE_ERROR = 1,
    FIFO_REMOVE_ERROR = 2,
    FIFO_OPEN_ERROR = 3,
    FIFO_READ_ERROR = 4,
    FIFO_WRITE_ERROR = 5
};