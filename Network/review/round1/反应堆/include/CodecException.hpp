#pragma once

#include <exception>
#include <string>

class CodecException : public std::exception {
    using string = std::string;

public:    
    CodecException(const char* m) : message(m) {}
    ~CodecException() = default;

    const char* what() const noexcept override {
        return message.c_str();
    }

   private:
    string message;
};
