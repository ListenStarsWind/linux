#pragma once

#include <string>
#include <unordered_map>

#include "client/CalculatorApp.hpp"
#include "codec.hpp"

class SessionAdapterBase {
   public:
    virtual ~SessionAdapterBase() = default;

    // 构建请求报文（返回字节流）
    virtual std::string send() = 0;

    // 接收响应报文（字节流），并解析
    // 返回值表示是否还需要继续接收(可能没有接收到一个完整的报文)
    virtual bool recv(std::string& response) = 0;
};

class SessionAdapter : public SessionAdapterBase {
    using hash = std::unordered_map<codec::message_type, CalculatorAppBase1*>;

   public:
    SessionAdapter() = default;

    template <typename... Apps>
    void register_recv_app(Apps*... apps) {
        (registerRecvApp(apps), ...);
    }

    template <typename... Apps>
    void register_send_app(Apps*... apps) {
        (registerSendApp(apps), ...);
    }

    void select_send_app(codec::message_type sendType) {
        _sendType = sendType;
    }

    void registerRecvApp(CalculatorAppBase1* app) {
        _recvApps.emplace(app->code(), app);
    }

    void registerSendApp(CalculatorAppBase1* app) {
        _sendApps.emplace(app->code(), app);
    }

    ~SessionAdapter() = default;

    std::string send() override {
        auto it = _sendApps.find(_sendType);
        if (it == _sendApps.end()) {
            throw std::logic_error("没有注册对应的发送对象");
        }
        return codec::pack(_sendType, it->second->code(), it->second->send());
    }

    bool recv(std::string& response) override {
        uint8_t version, type;
        std::string body;
        bool success = codec::unpack(response, &version, &type, &body);
        if (success == false) return false;
        if (version == 1) {
            auto it = _recvApps.find(static_cast<codec::message_type>(type));
            if (it == _recvApps.end()) {
                throw std::logic_error("收到未知类型报文，没有注册处理对象");
            }
            it->second->recv(body);
        } else {
            BOOST_LOG_TRIVIAL(warning) << "协议版本不一致" << std::endl;
        }

        return true;
    }

   private:
    hash _recvApps;
    hash _sendApps;
    codec::message_type _sendType;
};