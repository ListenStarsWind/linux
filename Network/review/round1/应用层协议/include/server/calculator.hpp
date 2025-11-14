#pragma once

#include <string>
#include "codec.hpp"
#include "counter.hpp"

class calculator{
    public:
    std::string operator()(std::string& message)
    {
        std::string playout;
        if(!codec::unpack(message, &playout)) return {};
        auto t = counter::from_string(playout);
        if(!t.is_ready()) return {};
        auto send_playout = t.work();
        return codec::pack(send_playout);
    }
};