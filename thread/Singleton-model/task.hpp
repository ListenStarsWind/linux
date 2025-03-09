#pragma once

#include <utility>
#include <cstdio>
#include<functional>
#include<unordered_map>

extern std::unordered_map<pthread_t, int> producers;

// C++11风格
class task
{
    private:
    typedef std::function<int(int, int)> func;
    typedef std::unordered_map<char, func> hash;
    hash counter
    {
        {{'+'},{[](int left, int right){return left + right;}}},
        {{'-'},{[](int left, int right){return left - right;}}},
        {{'*'},{[](int left, int right){return left * right;}}},
        {{'/'},{[](int left, int right){
            if(right == 0)
                throw "Zero cannot be a divisor";
            return left / right;
        }}},
    };

    typedef std::unordered_map<int, char> info;
    info op_
    {
        {{0}, {'+'}},
        {{1}, {'-'}},
        {{2}, {'*'}},
        {{3}, {'/'}},
    };

    public:
    task()
        :_left(0), _oper(op_[0]), _right(0)
        {}

    task(int left, int op, int right)
        :_left(left), _oper(op_[op%4]), _right(right)
        {}

    void push_info()const
    {
        printf("\033[31m[producer]info$ %d %c %d = ?\033[0m\n", _left, _oper, _right);
    }

    int operator()()
    {
        return counter[_oper](_left, _right);
    }

    private:
    int  _left;
    char _oper;
    int  _right;
};
