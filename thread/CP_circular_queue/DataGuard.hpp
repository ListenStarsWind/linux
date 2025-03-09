#pragma once

#include<functional>
#include <utility>


template<class T>
class DataGuard
{
    public:
    typedef std::function<void(const T& val)> function;

    template<class push_info, class pop_info>
    DataGuard(push_info in_log, pop_info out_log)
    :_data(T())
    ,_in_log(in_log)
    ,_out_log(out_log)
    {}

    template<class... Args>
    void push(Args&&... args)
    {
        T temp(std::forward<Args>(args)...);
        _in_log(temp);
        _data = std::move(temp);
    }

    T pop()
    {
        _out_log(_data);
        return _data;
    }

    private:
    T _data;
    function _in_log;
    function _out_log;
};