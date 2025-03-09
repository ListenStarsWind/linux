#pragma once

#include <utility>
#include <errno.h>
#include <stdio.h>
#include <string>

// C++98风格的可调用对象是仿函数

class task
{
public:
    task(int left, char op, int right)
        : _left(left), _op(op), _right(right)
    {
    }

    std::pair<int, int> operator()()
    {
        int _result = 0;
        int _errno = 0;
        switch (_op)
        {
        case '+':
            _result = _left + _right;
            break;
        case '-':
            _result = _left - _right;
            break;
        case '*':
            _result = _left * _right;
            break;
        case '/':
            if (_right == 0)
                _errno = 1;
            else
                _result = _left / _right;
            break;
        }
        return std::pair<int, int>(_result, _errno);
    }

    void producer_info()const
    {
        std::cout << "[producer]info: Generate a task: "<< _left << " " << _op << " " << _right << " = ?" <<std::endl;
    }

private:
    int _left;   // 左操作数
    char _op;     // 操作符
    int _right;  // 右操作数
};