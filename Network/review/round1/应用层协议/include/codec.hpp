#pragma once

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <string>
#include <format>

class codec {
    using string = std::string;

   public:
    static string pack(const string& playout) {
        string message;
        message += std::to_string(playout.size());
        message += codec::_delimiter;
        message += playout;
        message += codec::_message_delimiter;

        return message;
    }

static bool unpack(string& message, string* layout) {
    // 找到报头结尾：长度字段 + CRLF
    auto header_end = message.find(codec::_delimiter);
    if (header_end == string::npos) return false;

    // 负载开始的位置
    auto payload_begin = header_end + codec::_delimiter.size();

    BOOST_LOG_TRIVIAL(debug) << std::format("报文为: {}, pos:{}", message, header_end);

    // 解析报头里的长度字段
    auto payload_len = codec::stoi(message.substr(0, header_end));

    // 负载 + 结尾符（CRLF）
    auto payload_total = payload_len + codec::_message_delimiter.size();

    // 整个报文的总长度
    auto packet_size = payload_begin + payload_total;

    if (message.size() < packet_size) return false;

    // 截取负载
    *layout = message.substr(payload_begin, payload_len);

    // 移除一个完整报文
    BOOST_LOG_TRIVIAL(debug) << std::format("处理前的报文:{}", message);
    message.erase(0, packet_size);
    BOOST_LOG_TRIVIAL(debug) << std::format("处理后的报文:{}", message);

    return true;
}

   private:
    static int stoi(const string& num) {
        try {
            return std::stoi(num);
        } catch (...) {
            // 这条日志在我预想中应该不会触发, 但还是预防一下
            BOOST_LOG_TRIVIAL(error) << std::format("解包时报头参数无法解析为整型, 注意, 在预想中, 这种错误本不应该发生");
        }

        return 0;
    }

   private:
    inline static const string _delimiter = "\r\n";
    inline static const string _message_delimiter = "\r\n";
};
