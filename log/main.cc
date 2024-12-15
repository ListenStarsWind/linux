#include"log.hpp"

int main()
{
    wind::Log log(wind::category);
    log(wind::Info, "hello");
    log(wind::Debug, "hello");
    log(wind::Warning, "hello");
    log(wind::Error, "hello");
    log(wind::Fatal, "hello");
    return 0;
}