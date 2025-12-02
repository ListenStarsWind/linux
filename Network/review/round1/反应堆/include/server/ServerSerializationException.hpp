#pragma once

#include <string>
#include "SerializationException.hpp"
#include "proto/calculator.pb.h"

class ServerSerializationException : public SerializationException {
   public:

    ServerSerializationException(calc::CalcResponse_StatusCode code, const std::string& msg) : SerializationException(msg), code_(code) {}

    calc::CalcResponse_StatusCode code() const noexcept {
        return code_;
    }

   private:
    calc::CalcResponse_StatusCode code_;
};
