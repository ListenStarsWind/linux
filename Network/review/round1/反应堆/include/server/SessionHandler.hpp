#pragma once

#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <unordered_map>

#include "codec.hpp"
#include "proto/calculator.pb.h"
#include "server/ServerSerializationException.hpp"

class SessionHandler {
   public:
    std::string operator()(std::string& message) {
        std::string playout;
        uint8_t version = 0, type = 0;
        if (!codec::unpack(message, &version, &type, &playout)) return {};

        calc::CalcRequest quest_body;
        if (!quest_body.ParseFromString(playout)) {
            return SessionHandler::error(
                calc::CalcError_ErrorCode::CalcError_ErrorCode_REQUEST_DESERIALIZE_FAIL,
                "请求反序列化异常");
        }

        auto it = SessionHandler::func.find(std::string(quest_body.op()));
        if (it == SessionHandler::func.end()) {
            double result = 0;
            std::string running_error = "使用不存在的操作符";
            auto val = calc::CalcResponse_StatusCode::CalcResponse_StatusCode_INVALID_OPERATOR;

            return SessionHandler::serialize_or_error(result, val, running_error);
        }

        double result;
        std::string running_error = "没有错误";
        auto val = calc::CalcResponse_StatusCode::CalcResponse_StatusCode_SUCCESS;
        try {
            result = it->second(quest_body.left(), quest_body.right());
        } catch (ServerSerializationException& e) {
            val = e.code();
            running_error = e.what();
        }

        return SessionHandler::serialize_or_error(result, val, running_error);
    }

   private:
    static std::string error(calc::CalcError_ErrorCode val, const char* mes) {
        calc::CalcError error;
        error.set_session_code(val);
        error.set_description(mes);
        std::string resp_body;
        error.SerializeToString(&resp_body);
        return codec::pack(1, codec::message_type::ERROR, resp_body);
    }

    static std::string serialize_or_error(double result, calc::CalcResponse_StatusCode code,
                                          const std::string& message) {
        calc::CalcResponse resp;
        resp.set_result(result);
        resp.set_status_code(code);
        resp.set_message(message);

        std::string body;
        if (!resp.SerializeToString(&body)) {
            return SessionHandler::error(
                calc::CalcError_ErrorCode::CalcError_ErrorCode_RESPONSE_SERIALIZE_FAIL,
                "应答序列化失败");
        }

        return codec::pack(1, codec::message_type::RESPONSE, body);
    }

    static double add(double left, double right) {
        double result = left + right;
        if (!std::isfinite(result)) {
            throw ServerSerializationException(
                calc::CalcResponse_StatusCode::CalcResponse_StatusCode_OVERFLOW,
                std::format("加法溢出: {} + {}", left, right));
        }
        return result;
    }

    static double sub(double left, double right) {
        double result = left - right;
        if (!std::isfinite(result)) {
            throw ServerSerializationException(
                calc::CalcResponse_StatusCode::CalcResponse_StatusCode_OVERFLOW,
                std::format("减法溢出: {} - {}", left, right));
        }
        return result;
    }

    static double mul(double left, double right) {
        double result = left * right;
        if (!std::isfinite(result)) {
            throw ServerSerializationException(
                calc::CalcResponse_StatusCode::CalcResponse_StatusCode_OVERFLOW,
                std::format("乘法溢出: {} * {}", left, right));
        }
        return result;
    }

    static double div(double left, double right) {
        if (right == 0.0) {
            return (left >= 0.0) ? std::numeric_limits<double>::infinity()
                                 : -std::numeric_limits<double>::infinity();
        }
        double result = left / right;
        if (!std::isfinite(result)) {
            throw ServerSerializationException(
                calc::CalcResponse_StatusCode::CalcResponse_StatusCode_OVERFLOW,
                std::format("除法溢出: {} / {}", left, right));
        }
        return result;
    }

    inline static const std::unordered_map<std::string, double (*)(double, double)> func = {
        {"+", &SessionHandler::add},
        {"-", &SessionHandler::sub},
        {"*", &SessionHandler::mul},
        {"/", &SessionHandler::div},
    };
};
