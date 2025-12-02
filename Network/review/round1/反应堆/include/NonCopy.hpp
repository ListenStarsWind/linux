#pragma once

class NonCopy {
   protected:
    NonCopy() = default;
    NonCopy(const NonCopy&) = delete;
    const NonCopy& operator=(const NonCopy&) = delete;
};
