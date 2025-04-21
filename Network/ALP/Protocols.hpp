#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <jsoncpp/json/json.h>

#define FIELD_SEPARATOR ' '           // 负载中字段间的分隔符
#define HEADER_PAYLOAD_SEPARATOR '\n' // 报头和负载间的分隔符
#define MESSAGE_SEPARATOR '\n'        // 报文间的分隔符

#define DIVISION_BY_ZERO 1      // 除零错误
#define UNSUPPORTED_OPERATION 2 // 不支持的运算
#define RELIABLE 0

class result
{
public:
    result() : _val(0), _errno(0), _state(false) {}
    result(const int &val_, const int &errno_) : _val(val_), _errno(errno_), _state(true) {}
    result(const std::string &s) : _state(false) { init(s); }

    bool init(const std::string &s)
    {
#ifdef SERIALIZATION_MANUAL
        size_t pos = s.find(FIELD_SEPARATOR);
        if (pos == std::string::npos)
            return *this;
        std::string val_ = s.substr(0, pos);
        std::string errno_ = s.substr(pos + 1);
        _val = stoi(val_);
        _errno = stoi(errno_);

        _state = true;

        return *this;
#else
        Json::Value root;
        Json::Reader r;
        r.parse(s, root);
        _val = root["val"].asInt();
        _errno = root["errno"].asInt();
        _state = true;
        return *this;
#endif
    }

    operator std::string()
    {
#ifdef SERIALIZATION_MANUAL

        std::string temp;
        temp += std::to_string(_val);
        temp += FIELD_SEPARATOR;
        temp += std::to_string(_errno);

        return temp;
#else
        Json::Value root;
        Json::FastWriter w;
        root["val"] = _val;
        root["errno"] = _errno;
        return w.write(root);
#endif
    }

    operator bool() { return _state; }

    void print()
    {
        std::cout << "_val: " << _val << "  " << "_errno: ";
        if (_errno == DIVISION_BY_ZERO)
            std::cout << "DIVISION_BY_ZERO";
        else if (_errno == UNSUPPORTED_OPERATION)
            std::cout << "UNSUPPORTED_OPERATION";
        else if (_errno == RELIABLE)
            std::cout << "RELIABLE";
        std::cout << std::endl;
    }

private:
    int _val;
    int _errno;
    bool _state;
};

class expression
{
public:
    expression() : _x(0), _y(0), _op('+'), _state(false) {};
    expression(const int &x, char op, const int &y)
        : _x(x), _y(y), _op(op), _state(true)
    {
    }

    expression(const std::string &s)
        : _state(false) { init(s); }

    bool init(const std::string &s)
    {
#ifdef SERIALIZATION_MANUAL

        size_t pos1 = s.find(FIELD_SEPARATOR);
        if (pos1 == std::string::npos)
            return *this;
        size_t pos2 = s.find(FIELD_SEPARATOR, pos1 + 1);
        if (pos2 == std::string::npos)
            return *this;
        std::string x = s.substr(0, pos1);
        std::string op = s.substr(pos1 + 1, pos2 - (pos1 + 1));
        std::string y = s.substr(pos2 + 1);
        if (op.size() != 1) // 如果不为一说明格式错误,没有遵守协议, 抛出异常, 该报文已经可以放弃
            throw std::invalid_argument("Illegal parameter format");
        _x = std::stoi(x); // 当参数无法转换时, `stoi`也会抛出异常
        _op = op[0];
        _y = std::stoi(y);

        _state = true; // _state表示是否残缺, 异常表示格式错误

        return *this;

#else
        Json::Value v;
        Json::Reader r;
        r.parse(s, v);
        _x = v["x"].asInt();
        _op = v["op"].asInt();
        _y = v["y"].asInt();
        _state = true;
        return *this;
#endif
    }

    operator std::string()
    {
#ifdef SERIALIZATION_MANUAL
        std::string temp;
        temp += std::to_string(_x);
        temp += FIELD_SEPARATOR;
        temp += _op;
        temp += FIELD_SEPARATOR;
        temp += std::to_string(_y);

        return temp;
#else
        Json::Value root;
        root["x"] = _x;
        root["op"] = _op;
        root["y"] = _y;

        Json::FastWriter w;
        return w.write(root);
#endif
    }

    result operator()()
    {
        int val_ = 0;
        int erron_ = 0;
        switch (_op)
        {
        case '+':
            val_ = _x + _y;
            break;
        case '-':
            val_ = _x - _y;
            break;
        case '*':
            val_ = _x * _y;
            break;
        case '/':
            if (_y == 0)
                erron_ = DIVISION_BY_ZERO;
            else
                val_ = _x / _y;
            break;
        default:
            erron_ = UNSUPPORTED_OPERATION;
            break;
        }

        result temp(val_, erron_);
        return temp;
    }

    operator bool() { return _state; }

    void print()
    {
        std::cout << "_x: " << _x << "  " << "_y: " << _y << "  " << "_op: " << _op << std::endl;
    }

private:
    int _x;
    int _y;
    char _op;
    bool _state;
};

std::string pack(const std::string &message)
{
    std::string temp;
    temp += std::to_string(message.size());
    temp += HEADER_PAYLOAD_SEPARATOR;
    temp += message;
    temp += MESSAGE_SEPARATOR;

    return temp;
}

bool unpack(std::string &in, std::string *out)
{
    size_t pos = in.find(HEADER_PAYLOAD_SEPARATOR);
    if (pos == std::string::npos)
        return false;
    std::string len_ = in.substr(0, pos);
    int len = stoi(len_);
    size_t should = len + sizeof(MESSAGE_SEPARATOR);
    std::string payload = in.substr(pos + 1, should);
    if (payload.size() < should)            
        return false;

    // 存在足够长度的报文
    // 将这部分字符串移除

    in.erase(0, pos + strlen(HEADER_PAYLOAD_SEPARATOR) + should);
    *out = payload;

    return true;
}
