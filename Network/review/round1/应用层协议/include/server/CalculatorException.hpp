#pragma once

#include <stdexcept>
#include <string>
#include "proto/calculator.pb.h"

class CalculatorException : public std::runtime_error {
   public:

    CalculatorException(calc::CalcResponse_StatusCode code, const std::string& msg) : std::runtime_error(msg), code_(code) {}

    calc::CalcResponse_StatusCode code() const noexcept {
        return code_;
    }

   private:
    calc::CalcResponse_StatusCode code_;
};
