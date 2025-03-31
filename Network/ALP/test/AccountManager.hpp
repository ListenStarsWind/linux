#pragma once

#include "type.h"
#include <memory>
#include <exception>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>

#include <boost/bimap.hpp>

namespace wind
{

    class AccountManager_Base
    {
        typedef boost::bimap<UniqueIdType, struct in_addr> bimap;

    public:
        virtual void init(const string &UserListFile_ = string()) = 0;
        virtual std::string app_first_handshake(const std::string &message) = 0;
        struct in_addr query_left(const UniqueIdType &userID)
        {
            if (_accounts.left.find(userID) == _accounts.left.end())
            {
                throw std::runtime_error("User ID not found in accounts");
            }

            auto info = address_visualization(_accounts.left.at(userID));
            _log(Debug, "IP地址: %s", info.c_str());

            return _accounts.left.at(userID);
        }

        UniqueIdType create_account(const struct in_addr &sin_addr)
        {
            try
            {
                return query_right(sin_addr);
            }
            catch (const std::exception &e)
            {
                auto info = address_visualization(sin_addr);
                _log(Info, "正在创建新账号,  IP源: ", info.c_str());
                push(static_cast<UniqueIdType>(_accounts.size()), sin_addr);
                return query_right(sin_addr);
            }
        }

        UniqueIdType query_right(struct in_addr sin_addr)
        {
            if (_accounts.right.find(sin_addr) == _accounts.right.end())
            {
                throw std::runtime_error("User ID not found in accounts");
            }

            return _accounts.right.at(sin_addr);
        }

        template <class... Args>
        void push(Args &&...args)
        {
            _accounts.insert({std::forward<Args>(args)...});
        }

    protected:
        std::string address_visualization(const struct in_addr &sin_addr)
        {
            char ipbuff[32];
            inet_ntop(AF_INET, &sin_addr, ipbuff, sizeof(ipbuff));
            return ipbuff;
        }

        struct in_addr address_memorization(const string &message)
        {
            struct in_addr sin_addr;
            if (inet_aton(message.c_str(), &sin_addr) == 0)
            {
                throw std::runtime_error("Invalid IP address");
            }

            return sin_addr;
        }

    private:
        bimap _accounts;

    protected:
        Log &_log = Log::getInstance();
    };

    class AccountManager_server : public AccountManager_Base
    {
    public:
        void init(const string &UserListFile_) override
        {
            // 使用基类中纯虚函数的接口, 缺省值不能写在重写函数声明中
            const string &UserListFile = UserListFile_.empty() ? DEFAULT_FILE : UserListFile_;
            fstream Users(UserListFile.c_str());
            if (!Users.is_open())
            {
                _log(Fatal, "Failed to open user file");
                exit(FILE_OPEN_ERROR);
            }
            string user;
            while (getline(Users, user))
            {

                if (user.empty())
                    continue;

                size_t space = user.find(' ');
                if (space == std::string::npos)
                {
                    _log(Fatal, "Configuration file format error");
                    exit(FILE_FORMAT_ERROR);
                }
                string accoun_ = user.substr(0, space);
                string ip_ = user.substr(space + 1);

                UniqueIdType accoun = std::stoi(accoun_);

                // _log(Debug, "账号:%d", accoun);
                // _log(Debug, "地址:%s", ip_.c_str());

                struct in_addr sin_addr;
                memset(&sin_addr, 0, sizeof(sin_addr));

                try
                {
                    sin_addr = address_memorization(ip_);
                }
                catch (const std::exception &e)
                {
                    _log(Fatal, "%s", e.what());
                    exit(INVAL_CFG_ERROR);
                }

                AccountManager_Base::push(accoun, sin_addr);

                // if (user.empty())
                //     continue;
                // size_t space1 = user.find(' ');
                // if (space1 == string::npos)
                // {
                //     _log(Fatal, "Configuration file format error");
                //     exit(FILE_FORMAT_ERROR);
                // }
                // size_t space2 = user.find(' ', space1 + 1);
                // if (space2 == string::npos)
                // {
                //     _log(Fatal, "Configuration file format error");
                //     exit(FILE_FORMAT_ERROR);
                // }
                // string accoun_ = user.substr(0, space1);
                // string ip = user.substr(space1 + 1, (space2 - (space1 + 1)));
                // string port = user.substr(space2 + 1);

                // ssize_t port__ = stoi(port); // stoi可能抛出异常
                // if (port__ < 0 || port__ > 65535)
                // {
                //     _log(Fatal, "Invalid configuration parameter");
                //     exit(INVAL_CFG_ERROR);
                // }
                // uint16_t port_ = static_cast<uint16_t>(port__);
                // UniqueIdType accoun = stoi(accoun_);

                // // _log(Debug, "账号:%d", accoun);
                // // _log(Debug, "地址:%s", ip.c_str());
                // // _log(Debug, "端口:%d", port_);

                // EndpointIdType sock_in;
                // memset(&sock_in, 0, sizeof(sock_in));
                // sock_in.sin_family = AF_INET;
                // sock_in.sin_port = htons(port_);
                // if (inet_aton(ip.c_str(), &(sock_in.sin_addr)) == 0)
                // {
                //     _log(Fatal, "Invalid IP address");
                //     exit(INVAL_CFG_ERROR);
                // }

                // AccountManager_Base::push(accoun, sock_in);
            }
            Users.close();
        }

        // 账号管理层
        std::string app_first_handshake(const std::string &message) override
        {
            struct in_addr sin_addr;
            try
            {
                sin_addr = AccountManager_Base::address_memorization(message);
            }
            catch (const std::exception &e)
            {
                throw std::runtime_error("Handshake incomplete: invalid string received");
            }

            UniqueIdType temp = create_account(sin_addr);
            // _log(Debug, "服务层, 申请一个账号:%d", temp);
            return std::to_string(temp);
        }

    public:
        static AccountManager_server &getInstance()
        {
            static AccountManager_server inst;
            return inst;
        }

    private:
        AccountManager_server() {}
        AccountManager_server(const AccountManager_server &that) = delete;
        AccountManager_server &operator=(const AccountManager_server &that) = delete;

    private:
        static string DEFAULT_FILE;
    };

    string AccountManager_server::DEFAULT_FILE = "/home/wind/projects/Network/ALP/UsersForServer.txt";

    class AccountManager_client : public AccountManager_Base
    {
    public:
        void init(const string &UserListFile_) override
        {
            const string &UserListFile = UserListFile_.empty() ? DEFAULT_FILE : UserListFile_;
            fstream Users(UserListFile.c_str());
            if (!Users.is_open())
            {
                _log(Fatal, "Failed to open user file");
                exit(FILE_OPEN_ERROR);
            }
            string user;
            while (getline(Users, user))
            {

                if (user.empty())
                    continue;

                size_t space = user.find(' ');
                if (space == std::string::npos)
                {
                    _log(Fatal, "Configuration file format error");
                    exit(FILE_FORMAT_ERROR);
                }
                string accoun_ = user.substr(0, space);
                string ip_ = user.substr(space + 1);

                UniqueIdType accoun = std::stoi(accoun_);

                struct in_addr sin_addr;
                memset(&sin_addr, 0, sizeof(sin_addr));

                try
                {
                    sin_addr = address_memorization(ip_);
                }
                catch (const std::exception &e)
                {
                    _log(Fatal, "%s", e.what());
                    exit(INVAL_CFG_ERROR);
                }

                AccountManager_Base::push(accoun, sin_addr);
            }
            Users.close();
        }

        static AccountManager_client &getInstance()
        {
            static AccountManager_client inst;
            return inst;
        }

    private:
        AccountManager_client() {}
        AccountManager_client(const AccountManager_client &that) = delete;
        AccountManager_client &operator=(const AccountManager_client &that) = delete;

        private:
        static string DEFAULT_FILE;
    };

    string AccountManager_client::DEFAULT_FILE = "/home/wind/projects/Network/ALP/UsersForClient.txt";

}