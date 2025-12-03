# pragma once

#include <stdexcept>

// 应用层和表示层过度时的异常 (有关序列化和反序列化的异常)
class SerializationException : public std::runtime_error {
public:
    explicit SerializationException(const std::string& msg)
        : std::runtime_error(msg) {}
};
