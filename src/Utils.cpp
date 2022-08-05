#include "Command.h"
#include "Request.h"
#include "Utils.h"

#include <cstring>
#include <string>

namespace radio
{

Request createStatusRequest(const std::string& address)
{
    protocol_msg_t msg;
    protocol_msg_init(&msg);

    msg.msg_type = PROTO_MSG_READ_STATUS;

    return Request{ std::move(msg), address };
}

Request createCommandRequest(Command command, const std::string& address)
{
    protocol_msg_t msg;
    protocol_msg_init(&msg);

    msg.msg_type = PROTO_MSG_COMMAND;
    msg.payload.command = commandToProtocolCmd(command);

    return Request{ std::move(msg), address };
}

protocol_cmd_t commandToProtocolCmd(Command command)
{
    switch (command)
    {
    case Command::AllDown:
        return PROTO_CMD_ALL_DOWN;

    case Command::AllUp:
        return PROTO_CMD_ALL_UP;

    case Command::Shutter1Down:
        return PROTO_CMD_SHUTTER_1_DOWN;

    case Command::Shutter1Up:
        return PROTO_CMD_SHUTTER_1_UP;

    case Command::Shutter2Down:
        return PROTO_CMD_SHUTTER_2_DOWN;

    case Command::Shutter2Up:
        return PROTO_CMD_SHUTTER_2_UP;
    }

    // assert(false);

    return PROTO_CMD_NOP;
}

std::string deviceIndexToAddress(const uint8_t index)
{
    if (index > 9) {
        return {};
    } 

    std::string s{ "SMRR0" };
    s[4] = '0' + index;

    return s;
}

}

std::string Utils::pgmToStdString(PGM_P str)
{
    const auto len = strlen_P(str);
    std::string ss(len, 0);
    memcpy_P(&ss[0], str, len);
    return ss;
}
