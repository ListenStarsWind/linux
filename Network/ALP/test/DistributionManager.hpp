#pragma once

#include"type.h"
#include"AccountManager.hpp"

namespace wind
{
    class DistributionManager_Base
    {
        protected:
        DistributionManager_Base() : _manager(AccountManager_server::getInstance()) , _log(Log::getInstance()){}

        std::string unpack(const Pack_Message& message)
        {
            enum TaskCode task = message._header._task;
            switch(task)
            {
                case CREATE_NEW_ACCOUNT: return go_AccountManager(message._payload);
            }
        }

        std::string go_AccountManager(const string& message)
        {
            size_t i = message.find('\n');
            if(i != std::string::npos)
            {
                // _log(Debug, "任务: %s", "创建一个账号");
                return _manager.app_first_handshake(message.substr(i+1));
            }

            throw std::runtime_error("Handshake incomplete: invalid string received");
        }

        private:
        AccountManager_Base& _manager;
        protected:
        Log& _log;
    };

    class DistributionManager_server : public DistributionManager_Base
    {
        public:
        static DistributionManager_server &getInstance()
        {
            static DistributionManager_server inst;
            return inst;
        }

        std::string unpack(const Pack_Message& message)
        {
             return DistributionManager_Base::unpack(message);
        }

    private:
    DistributionManager_server() {}
    DistributionManager_server(const DistributionManager_server &that) = delete;
    DistributionManager_server &operator=(const DistributionManager_server &that) = delete;
    };
}