#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string>
#include <unordered_map>
#include "log.hpp"

namespace wind
{
    // 与共享内存有关的相关常量
    enum ShmGetMode
    {
        PreferExisting = IPC_CREAT,             // 表示优先复用已有资源
        ForceNew = IPC_CREAT | IPC_EXCL | 0666, // 强制创建新的共享内存
        size = 4096,
        proj_id = 0x1246,
        remove = IPC_RMID,
        statu = IPC_STAT,
        pathname
    };

    // 为无法在枚举中映射的成员建立映射关系
    const std::unordered_map<ShmGetMode, const char *> ShmGetModeToCString =
        {
            {pathname, "/home/wind"}};

    class channel
    {
        enum error
        {
            ftok_error = 1,
            shmget_error = 2,
            attach_error
        };

        const char *toCStrung(ShmGetMode mode)
        {
            Log log;
            auto it = ShmGetModeToCString.find(mode);
            if (it == ShmGetModeToCString.end())
            {
                log(Fatal, "invalid mapping!");
                return "Unknown";
            }
            else
            {
                return it->second;
            }
        }

    public:
        channel(ShmGetMode Mode, ShmGetMode Size = size, ShmGetMode Path = pathname, ShmGetMode KeyHint = proj_id)
            : _mode(Mode), _ptr(nullptr), _size(Size)
        {
            key_t key = getKey(Path, KeyHint);
            // sleep(2);
            _id = getShmid(key, Size, Mode);
            // sleep(2);
            _ptr = getShmAttach(_id);
            // sleep(5);
        }

        ~channel()
        {
            if (_mode == ForceNew)
            {
                int n = shmdt(_ptr);
                if (n == -1)
                {
                    log(Error, "failed to detach shared memory:%s", strerror(errno));
                }
                else
                {
                    log(Info, "shared memory detached successfully");
                }
                n = shmctl(_id, remove, nullptr);
                if (n == -1)
                {
                    log(Error, "failed to delete shared memory:%s", strerror(errno));
                }
                else
                {
                    log(Info, "delete shared memory successfully");
                }
            }
        }

        const char* read()
        {
            return _ptr;
        }

        ssize_t write(const char* str)
        {
            size_t len = strlen(str) + 1;
            if(len > _size)
            {
                log(Error, "too much data");
                memcpy(_ptr, str, _size);
                return _size;
            }
            else
            {
                 memcpy(_ptr, str, len);
                 return len;
            }
        }

        struct shmid_ds getStatu()
        {
            struct shmid_ds buf;
            int n = shmctl(_id, statu, &buf);
            return buf;
        }

    private:
        key_t getKey(ShmGetMode Path, ShmGetMode KeyHint)
        {
            int n = ftok(toCStrung(Path), KeyHint);
            if (n < 0)
            {
                log(Fatal, "filed ftok:%s", strerror(errno));
                exit(ftok_error);
            }
            else
            {
                log(Info, "succeeded ftok, the key is 0x%x", n);
                return n;
            }
        }

        int getShmid(key_t key, ShmGetMode Size, ShmGetMode Mode)
        {
            int n = shmget(key, Size, Mode);
            if (n == -1)
            {
                log(Fatal, "create share memory error:%s", strerror(errno));
                exit(shmget_error);
            }
            else
            {
                log(Info, "create share memory success, the shmid is %d", n);
                return n;
            }
        }

        char *getShmAttach(int id)
        {
            char *ptr = (char *)shmat(id, nullptr, 0);
            if (ptr == (void *)-1)
            {
                log(Fatal, "shared memory attach failed:%s", strerror(errno));
                exit(attach_error);
            }
            else
            {
                log(Info, "shared memory attach success, the mapped address is %p", ptr);
                return ptr;
            }
        }

        Log log;
        ShmGetMode _mode;
        int _id;      // 共享内存标识符
        char *_ptr;   // 共享内存的起始地址
        size_t _size; // 共享内存的大小
    };
}