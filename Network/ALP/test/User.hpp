#pragma once

#include"type.h"

namespace wind
{
    class User
    {
        operator EndpointIdType()
        {
            return _endpointId;
        }
        private:
        EndpointIdType _endpointId;
    };
}