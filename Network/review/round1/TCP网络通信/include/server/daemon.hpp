#pragma once

#include <signal.h>
#include <unistd.h>  // fork, setsi 
#include <cstdlib>     // exit
#include <string>
#include <fcntl.h> // open

inline void daemon(const std::string& cwd = "/") {
    ::signal(SIGCLD, SIG_IGN);
    ::signal(SIGPIPE, SIG_IGN);
    ::signal(SIGSTOP, SIG_IGN);

    if(fork() > 0) ::exit(0);

    ::setsid();

    ::chdir(cwd.c_str());

    int fd = open("/dev/null", O_RDWR);

    if(fd > 0)
    {
        ::dup2(fd, 0);
        ::dup2(fd, 1);
        ::dup2(fd, 2);
        ::close(fd);
    }
}
