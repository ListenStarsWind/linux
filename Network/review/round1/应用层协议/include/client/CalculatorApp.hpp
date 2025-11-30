#pragma once
#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <string>

#include "client/ClientSerializationException.hpp"
#include "codec.hpp"
#include "proto/calculator.pb.h"

// 以下继承体系, 末尾数字表示版本
class CalculatorAppBase1 {
   public:
    CalculatorAppBase1(double left, const std::string& op, double right)
        : _left(left), _op(op), _right(right) {}

    virtual std::string send() = 0;
    virtual void recv(std::string body) = 0;
    virtual codec::message_type code() = 0;
    uint8_t version() const {
        return _version;
    }

    virtual ~CalculatorAppBase1() = default;

   protected:
    [[noreturn]] static void invalid_call() {
        throw std::logic_error("这个函数不该被调用");
    }

   protected:
    double _left;
    std::string _op;
    double _right;
    inline static const uint8_t _version = 1;
};

class CalculatorRequestApp1 : public CalculatorAppBase1 {
   public:
    CalculatorRequestApp1(double left, const std::string& op, double right)
        : CalculatorAppBase1(left, op, right) {}

    ~CalculatorRequestApp1() override = default;

    std::string send() override {
        calc::CalcRequest request;
        request.set_left(_left);
        request.set_op(_op);
        request.set_right(_right);
        std::string body;
        if (!request.SerializeToString(&body)) {
            throw ClientSerializationException("请求反序列化失败");
        }
        BOOST_LOG_TRIVIAL(debug) << std::format("成功拼接请求负载: {}{}{}\n", _left, _op, _right);
        return body;
    }

    codec::message_type code() override {
        static const auto code = codec::message_type::REQUEST;
        return code;
    }

    void recv(std::string body) override {
        (void)body;
        CalculatorAppBase1::invalid_call();
    }
};

// Pesponse 表示报文类型
class CalculatorPesponseApp1 : public CalculatorAppBase1 {
   public:
    CalculatorPesponseApp1(double left, const std::string& op, double right)
        : CalculatorAppBase1(left, op, right) {}

    ~CalculatorPesponseApp1() override = default;

    void recv(std::string body) override {
        calc::CalcResponse res;
        if (!res.ParseFromString(body)) {
            throw ClientSerializationException("应答反序列化失败");
        }
        switch (res.status_code()) {
            case calc::CalcResponse_StatusCode::CalcResponse_StatusCode_SUCCESS: {
                BOOST_LOG_TRIVIAL(info) << std::format("{}{}{}={}\n", _left, _op, _right, res.result());
                break;
            }
            case calc::CalcResponse_StatusCode::CalcResponse_StatusCode_OVERFLOW: {
                BOOST_LOG_TRIVIAL(warning) << res.message() << std::endl;
                break;
            }
            case calc::CalcResponse_StatusCode::CalcResponse_StatusCode_INVALID_OPERATOR: {
                BOOST_LOG_TRIVIAL(warning) << std::format("{}: {}\n", res.message(), _op);
                break;
            }
            default: {
                BOOST_LOG_TRIVIAL(warning) << format("未知的状态码: {}\n",
                                    calc::CalcResponse_StatusCode_Name(res.status_code()));
                break;
            }
        }
    }

    codec::message_type code() override {
        static const auto code = codec::message_type::RESPONSE;
        return code;
    }

    std::string send() override {
        CalculatorAppBase1::invalid_call();
        return {};
    }
};

class CalculatorErrorApp1 : public CalculatorAppBase1 {
   public:
    CalculatorErrorApp1(double left, const std::string& op, double right)
        : CalculatorAppBase1(left, op, right) {}

    ~CalculatorErrorApp1() override = default;

    void recv(std::string body) override {
        calc::CalcError error;
        if (!error.ParseFromString(body)) {
            throw ClientSerializationException("错误类型报文负载反序列化失败");
        }
        switch (error.session_code()) {
            case calc::CalcError::ErrorCode::CalcError_ErrorCode_UNKNOWN_ERROR: {
                BOOST_LOG_TRIVIAL(info) << "错误类型未知" << std::endl;
                break;
            }
            case calc::CalcError::ErrorCode::CalcError_ErrorCode_REQUEST_DESERIALIZE_FAIL: {
                BOOST_LOG_TRIVIAL(warning) << std::format("服务端报告: {}\n", error.description());
                break;
            }
            case calc::CalcError::ErrorCode::CalcError_ErrorCode_RESPONSE_SERIALIZE_FAIL: {
                BOOST_LOG_TRIVIAL(warning) << std::format("服务端报告: {}\n", error.description());
                break;
            }
            default: {
                BOOST_LOG_TRIVIAL(warning) << std::format("错误状态码是未知的: {}\n",
                                         calc::CalcError::ErrorCode_Name(error.session_code()));
                break;
            }
        }
    }

    codec::message_type code() override {
        static const auto code = codec::message_type::ERROR;
        return code;
    }

    std::string send() override {
        CalculatorAppBase1::invalid_call();
        return {};
    }
};