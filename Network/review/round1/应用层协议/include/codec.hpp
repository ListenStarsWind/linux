#pragma once

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <string>
#ifdef USE_PROTOBUF
#include <vector>
#endif
#include <format>

class codec {

#ifndef USE_PROTOBUF
    using buffer = std::string;
    #else
    using buffer = std::vector<char>;
    #endif

   public:
    static buffer pack(const buffer& payload) {
        buffer message;
        codec::append(message, payload.size());
        codec::append(message, codec::_delimiter);
        codec::append(message, payload);
        codec::append(message, codec::_message_delimiter);

        return message;
    }

    static bool unpack(buffer& message, buffer* payload) {
        // 找到报头结尾：长度字段 + CRLF
        auto header_end = codec::buffer_find(message, codec::_delimiter);
        if (header_end == codec::npos) return false;

        // 负载开始的位置
        auto payload_begin = header_end + codec::_delimiter.size();

        // BOOST_LOG_TRIVIAL(debug) << std::format("报文为: {}, pos:{}", message, header_end);

        // 解析报头里的长度字段
        auto message_headers = codec::substr(message, 0, header_end);
        auto payload_len = codec::stoi(message_headers);

        // 负载 + 结尾符（CRLF）
        auto payload_total = payload_len + codec::_message_delimiter.size();

        // 整个报文的总长度
        auto packet_size = payload_begin + payload_total;

        if (message.size() < packet_size) return false;

        // 截取负载
        payload->clear();
        char* begin = message.data() + payload_begin;
        char* end = begin + payload_len;
        payload->insert(payload->end(), begin, end);

        // 移除一个完整报文
        // BOOST_LOG_TRIVIAL(debug) << std::format("处理前的报文:{}", message);
        codec::erase_front(message, packet_size);
        // BOOST_LOG_TRIVIAL(debug) << std::format("处理后的报文:{}", message);

        return true;
    }

   private:
    static int stoi(const std::string& num) {
        try {
            return std::stoi(num);
        } catch (...) {
            // 这条日志在我预想中应该不会触发, 但还是预防一下
            BOOST_LOG_TRIVIAL(error) << std::format(
                "解包时报头参数无法解析为整型, 注意, 在预想中, 这种错误本不应该发生");
        }

        return 0;
    }

// 负载可能是不可读的, 但应用层报头都是可读的, 所以关于报头的构建, 不管底层如何,
// 采用的都是先把单个元素变成字符串, 再把字符串追加到buffer对象中
#ifndef USE_PROTOBUF
    template <class T>
    static void append(buffer& dest, const T& src) {
        dest += std::to_string(src);
    }

    static size_t buffer_find(const buffer& b, const std::string& target) {
        return b.find(target);
    }

    static std::string substr(const buffer& b, size_t start, size_t size)
    {
        return b.substr(start, size);
    }

    static void erase_front(buffer& b, size_t len)
    {
        b.erase(0, len);
    }
#else
    template <class T>
    static void append(buffer& dest, const T& src) {
        auto copy = std::to_string(src);
        dest.insert(dest.end(), copy.data(), (copy.data() + copy.size()));
    }

    // 针对负载的特化
    static void append(buffer& dest, const buffer& src) {
        // 直接追加防止to_string 丢失非可见二进制信息
        dest.insert(dest.end(), copy.data(), (copy.data() + copy.size()));
    }

    static size_t buffer_find(const buffer& b, const std::string& target) {
        if (target.empty()) return 0;
        if (b.size() < target.size()) return codec::npos;

        size_t start = 0;
        size_t end = b.size() - target.size();
        for (size_t i = start; i <= end; ++i) {
            bool match = true;
            for (size_t j = 0; j < target.size(); ++j) {
                if (b[i + j] != target[j]) {
                    math = false;
                    break;
                }
            }
            if (match) return i;
        }
         return codec::npos;
    }

    static std::string substr(const buffer& b, size_t start, size_t size)
    {
        char* begin = b.data() + start;
        char* end = begin + size;
        return {begin, end};
    }
    
    static void erase_front(buffer& b, size_t len)
    {
        char* begin = b.data() + len;
        char* end = b.data() + b.size();
        buffer temp{begin, end};
        b.swap(temp);
    }
#endif

   private:
    inline static const std::string _delimiter = "\r\n";
    inline static const std::string _message_delimiter = "\r\n";
    inline static const size_t npos = -1;
};
