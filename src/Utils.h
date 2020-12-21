#pragma once

#include "Command.h"
#include "Request.h"
#include "radio_protocol.h"

#include <cstdint>

namespace radio
{

Request createStatusRequest(const std::string& address);
Request createCommandRequest(Command command, const std::string& address);
protocol_cmd_t commandToProtocolCmd(Command command);
std::string deviceIndexToAddress(uint8_t index);

}
