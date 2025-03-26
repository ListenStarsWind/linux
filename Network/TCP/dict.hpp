#pragma once
#include <string>
#include <fstream>
#include <unordered_map>

#include <iostream>

#define DICTPIFE "dict.txt"

namespace wind
{
    class dict
    {
        typedef std::string string;
        typedef std::fstream fstream;
        typedef std::unordered_map<string, string> unordered_map;

    public:
        static dict &getInstance()
        {
            static dict inst;
            return inst;
        }

        const string &translate(const string &s)
        {
            if (hash.find(s) != hash.end())
                return hash[s];
            else
                return _s;
        }

    private:
        dict()
        {
            fstream fs(DICTPIFE);
            while (fs)
            {
                string kv;
                getline(fs, kv);
                size_t space1 = kv.find(' ');
                size_t space2 = kv.find('\n', space1 + 1);
                string k = kv.substr(0, space1);
                string v = kv.substr(space1 + 1, space2 - (space1 + 1));
                hash[k] = v;
            }
        }

        void test()
        {
            for (const auto &[m, n] : hash)
            {
                std::cout << m << ":" << n << std::endl;
            }
        }

    private:
        unordered_map hash;
        string _s = "unknow";
    };

}