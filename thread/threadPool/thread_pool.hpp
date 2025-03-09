#pragma once

#include <pthread.h>
#include <string>
#include <vector>
#include <queue>

struct thread
{
    pthread_t tid;
    // 线程的其它属性
    std::string name;
};

template <class task_>
class thread_pool
{
    typedef thread_pool<task_> self;

public:
    thread_pool(int num = 5) : _threads(num)
    {
        pthread_mutex_init(&_mutex, nullptr);
        pthread_cond_init(&_cond, nullptr);
    }

    ~thread_pool()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_cond);
    }

    void start()
    {
        for (int i = 0; i < _threads.size(); ++i)
        {
            _threads[i].name = "thread-" + std::to_string(i + 1);
            pthread_create(&(_threads[i].tid), nullptr, behavior, this);
        }
    }

    template <class... Args>
    void push(Args &&...args)
    {
        pthread_mutex_lock(&_mutex);
        _tasks.emplace(std::forward<Args>(args)...);
        pthread_cond_signal(&_cond);
        pthread_mutex_unlock(&_mutex);
    }

private:
    static void *behavior(void *p_)
    {
        thread_pool<task_> *p = static_cast<thread_pool<task_> *>(p_);
        while (true)
        {
            pthread_mutex_lock(&p->_mutex);
            while (p->_tasks.empty())
            {
                pthread_cond_wait(&p->_cond, &p->_mutex);
            }
            auto t = p->_tasks.front();
            p->_tasks.pop();
            pthread_mutex_unlock(&p->_mutex);
            t.push_info();
            try
            {
                int result = t();
                printf("\033[32m[consumer]info$ %d\033[0m\n", result);
            }
            catch (const char *temp)
            {
                printf("\033[32m[consumer]info$ %s\033[0m\n", temp);
            }
        }
    }

private:
    std::vector<thread> _threads;
    std::queue<task_> _tasks;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
};