#pragma once

#include "SerializationException.hpp"

class ClientSerializationException : public SerializationException {
public:
    explicit ClientSerializationException(const std::string& msg)
        : SerializationException(msg) {}
    // 客户端不需要错误码，捕获即可决定重试或退出
};
