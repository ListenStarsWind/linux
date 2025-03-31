#pragma once

#include "type.h"
#include "log.hpp"
#include "AccountManager.hpp"
#include "DistributionManager.hpp"

namespace wind
{
    class PacketProcessor_Base
    {
        typedef unsigned int uint;

    public:
        PacketProcessor_Base() : _manager(AccountManager_server::getInstance()), _distr(DistributionManager_server::getInstance()) {};

        virtual std::string app_first_handshake(const std::string &message) = 0;
        // #报头(源$目的地$任务号)\nIP地址
        std::string unpack(const string &message)
        {
            size_t i = message.find('#');
            if (i != std::string::npos)
            {
                size_t j = message.find('$', i + 1);
                if (j != std::string::npos)
                {
                    size_t k = message.find('$', j + 1);
                    if (k != std::string::npos)
                    {
                        size_t end = message.find('\n', k + 1);
                        if (end != std::string::npos)
                        {
                            Pack_Message message_;
                            message_._header._source = std::stoi(message.substr(i + 1, j - (i + 1)));
                            message_._header._destination = std::stoi(message.substr(j + 1, k - (j + 1)));
                            message_._header._task = static_cast<TaskCode>(std::stoi(message.substr(k + 1, end - (k + 1))));
                            message_._payload = message.substr(end);

                            // _log(Debug, "源账号: %d", message_._header._source);
                            // _log(Debug, "目的账号: %d", message_._header._destination);

                            return _distr.unpack(message_);
                        }
                    }
                }
            }

            throw std::runtime_error("Handshake incomplete: invalid string received");
        }

        // virtual string pack(const string& app_laye_payload) = 0;
    private:
        Pack_Message _message;
        DistributionManager_server &_distr;

    protected:
        AccountManager_Base &_manager;
        Log &_log = Log::getInstance();
    };

    class PacketProcessor_Server : public PacketProcessor_Base
    {
    public:
        // 解包层     第一次握手的形式  do_#报头(源$目的地$任务号)\nIP地址
        std::string app_first_handshake(const std::string &message) override
        {
            size_t i = message.find(do_);
            if (i != std::string::npos)
            {
                return unpack(message.substr(i + do_.size()));
                // return {_manager.app_first_handshake(hello, client), done_};
            }

            throw std::runtime_error("Handshake incomplete: invalid string received");
        }

        // string pack(const string& app_laye_payload) override
        // {

        // }

        static PacketProcessor_Server &getInstance()
        {
            static PacketProcessor_Server inst;
            return inst;
        }

    private:
        PacketProcessor_Server() {}
        PacketProcessor_Server(const PacketProcessor_Server &that) = delete;
        PacketProcessor_Server &operator=(const PacketProcessor_Server &that) = delete;

        static const std::string do_;
        static const std::string done_;
    };

    const std::string PacketProcessor_Server::do_ = "do";
    const std::string PacketProcessor_Server::done_ = "done";
}
