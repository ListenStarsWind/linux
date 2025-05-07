#pragma once

#include"Protocols.hpp"


// 上层业务
class calculator
{
    public:
    std::string operator()(std::string& package)
    {
        std::string payload; expression t;
        if(!unpack(package, &payload)) return "";
        t.init(payload); if(!t) return "";
        auto e = t();
        return std::move(pack(e));
    }
};