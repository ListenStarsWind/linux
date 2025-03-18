#include"udpserver.hpp"
#include<iostream>
#include<string>
#include<vector>

using namespace std;


string func(const string& str, const string& ip, const uint16_t& port)
{
    cout << "["<<ip<<":"<<port<<"]$ "<<str<<endl;


    string result("server: ");
    result += str;
    result += "\n";
    return result;
}

bool SecurityCheck(const string& str)
{
    static const vector<string> blacklists =
    {
        "rm", "sudo", "yum", "install", "uninstall",
        "mv", "cp",  "kill", "unlink"
    };

    for(const auto& e : blacklists)
    {
        if(string::npos != str.find(e))
            return true;
    }

    return false;
}

string ExcuteCommand(const string& str)
{
    if(SecurityCheck(str))
    {
        return "Permission denie\n";
    }

    FILE* fp = popen(str.c_str(), "r");
    if(fp == nullptr)
    {
        string err(strerror(errno));
        cout<<"popen error: "<<err<<endl;
        return err;
    }

    string result;
    char buffer[4096];
    while(true)
    {
        char* str = fgets(buffer, sizeof(buffer), fp);
        if(str == nullptr) break;
        result += str;
    }

    // 就不考虑返回状态了
    int status = fclose(fp);

    return result;

}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout<<"Please enter the correct parameters."<<std::endl;
        exit(0);
    }
    wind::udpserver svr(std::stoi(*(argv + 1) + 1));
    svr.init();
    svr.run();
    return 0;
}