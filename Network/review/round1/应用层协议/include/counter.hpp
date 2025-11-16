#pragma once
#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <format>
#include <limits>     // 无穷大
#include <stdexcept>  //std::invalid_argument
#include <string>

class counter {
    using string = std::string;
    using self_t = counter;

   public:
    counter() : _state(false) {}
    counter(double x, char op, double y) : _x(x), _op(op), _y(y), _state(true) {}

    string to_string() {
        if (_state == false) {
            BOOST_LOG_TRIVIAL(warning) << std::format("正在对未初始化的对象序列化");
        }
        string temp;
        temp += std::to_string(_x);
        temp += counter::_space;
        temp += _op;
        temp += counter::_space;
        temp += std::to_string(_y);

        return temp;
    }

    static self_t from_string(const string& s) {
        self_t result;
        auto space = counter::_space;
        auto pos1 = s.find(space);
        if (pos1 == string::npos) return result;
        auto pos2 = s.find(space, pos1 + 1);
        if (pos2 == string::npos) return result;
        auto x = s.substr(0, pos1);
        auto op = s.substr(pos1 + 1, pos2 - (pos1 + 1));
        auto y = s.substr(pos2 + 1);
        if (op.size() != 1) throw std::invalid_argument("操作符的长度不为一");
        bool flag = true;
        for (auto c : counter::_ops) {
            if (c == op[0]) {
                flag = false;
                break;
            }
        }
        if (flag) throw std::invalid_argument("使用了不支持的操作符");

        // BOOST_LOG_TRIVIAL(debug) << std::format("x:{}, op:{}, y:{}", x, op, y);
        
        double x_, y_;
        char op_ = op[0];

        x_ = counter::stod(x);
        y_ = counter::stod(y);

        result = self_t(x_, op_, y_);

        return result;
    }

    string work() {
        if (_state == false) {
            BOOST_LOG_TRIVIAL(warning) << std::format("正在对未初始化的对象进行计算");
        }

        if (_op == '/' && _y == 0) {
            if (_x > 0) _result = +counter::inf;
            if (_x < 0) _result = -counter::inf;
            if (_x == 0) _result = counter::inf;
        } else {
            double result;
            switch (_op) {
                case '+':
                    result = _x + _y;
                    break;
                case '-':
                    result = _x - _y;
                    break;
                case '*':
                    result = _x * _y;
                    break;
                case '/':
                    result = _x / _y;
                    break;
            }

            _result = result;
        }

        return std::format("{}{}{}={}", _x, _op, _y, _result);
    }

    bool is_ready() {
        return _state;
    }

    void log() {
        if (_state == false) {
            BOOST_LOG_TRIVIAL(warning) << std::format("尝试对未初始化的对象输出日志");
            return;
        }

        BOOST_LOG_TRIVIAL(info) << std::format("{}{}{}={}", _x, _op, _y, _result);
    }

   private:
    static double stod(const string& s) {
        try {
            size_t pos = 0;
            auto n = std::stod(s, &pos);
            if (pos != s.size()) {
                BOOST_LOG_TRIVIAL(warning) << std::format("操作数没有完全消费完");
            }
            return n;
        } catch (...) {
            throw std::invalid_argument("操作数无法转化为double");
        }
    }

   private:
    double _x;
    char _op;
    double _y;
    bool _state;
    double _result;

   public:
    inline static const double inf = std::numeric_limits<double>::infinity();
    inline static const char _ops[4] = {'+', '-', '*', '/'};
    inline static const char _space = ' ';
};
