#pragma once

#include <arpa/inet.h>

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <format>
#include <string>

#include "CodecException.hpp"

// +-------------------------------------------+
// | Magic (2 bytes)    固定值 0xCAFE          |
// +-------------------------------------------+
// | Version (1 byte)   协议版本               |
// +-------------------------------------------+
// | MsgType (1 byte)   请求 / 响应 / 错误等    |
// +-------------------------------------------+
// | BodyLength (4 bytes, network byte order)  |
// +-------------------------------------------+
// | Body (variable)    protobuf 序列化内容     |
// +-------------------------------------------+
class codec {
    public:
    enum message_type { REQUEST = 0, RESPONSE = 1, ERROR = 3 };

    private:
    using string = std::string;

   public:
    static string pack(uint8_t version, uint8_t type, const string& payload) {
        uint32_t body_len = payload.size();

        string buf;
        buf.reserve(2 + 1 + 1 + 4 + body_len);

        // magic (2 bytes)
        uint16_t n_magic = htons(codec::magic);
        buf.append(reinterpret_cast<const char*>(&n_magic), sizeof(n_magic));

        // version (1 byte)
        buf.push_back(static_cast<char>(version));

        // type (1 byte)
        buf.push_back(static_cast<char>(type));

        // payload length (4 bytes)
        uint32_t n_len = htonl(body_len);
        buf.append(reinterpret_cast<const char*>(&n_len), sizeof(n_len));

        // payload (binary)
        buf += payload;

        return buf;
    }

    static bool unpack(string& message, uint8_t* version, uint8_t* type, string* payload) {
        constexpr size_t HEADER_SIZE = 2 + 1 + 1 + 4;

        if (message.size() < HEADER_SIZE) return false;  // 数据不够一帧头部

        const char* buf = message.data();

        // ----- 1. magic -----
        uint16_t n_magic = 0;
        ::memcpy(&n_magic, buf, sizeof(n_magic));
        uint16_t magic = ::ntohs(n_magic);

        if (magic != codec::magic) {
            throw CodecException("接收到不能被识别的报文");
        }

        // ----- 2. version -----
        ::memcpy(version, buf + 2, 1);

        // ----- 3. type -----
        ::memcpy(type, buf + 3, 1);

        // ----- 4. payload length -----
        uint32_t n_len = 0;
        ::memcpy(&n_len, buf + 4, 4);
        uint32_t len = ntohl(n_len);

        if (message.size() < HEADER_SIZE + len) return false;  // 数据还不完整，继续等待

        // ----- 5. payload -----
        payload->assign(buf + HEADER_SIZE, len);

        // ----- 6. 移除已解析的数据（关键，避免粘包问题） -----
        message.erase(0, HEADER_SIZE + len);

        return true;
    }

   private:
    inline static uint16_t magic = 0xCAFE;
    inline static uint8_t version = 0x01;
};
