#pragma once

#include "Result.h"
#include "radio_protocol.h"
#include <vector>

namespace radio
{

struct Response
{
    Result result;
    std::vector<protocol_msg_t> messages;
};

}
