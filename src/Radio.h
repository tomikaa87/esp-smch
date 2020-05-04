#pragma once

#include "Command.h"
#include "Logger.h"
#include "nrf24.h"
#include "radio_protocol.h"
#include "Response.h"
#include "Request.h"

#include <vector>

namespace radio
{

class Radio
{
public:
    Radio(uint8_t transceiverChannel);

    bool sendCommand(Command command, const std::string& address);
    bool readStatus(const std::string& address);

    void task();

private:
    const Logger _log{ "Radio" };

    enum class InterruptResult
    {
        Timeout,
        DataSent,
        DataReceived,
        PacketLost
    };

    enum class TransceiverMode
    {
        PrimaryTransmitter,
        PrimaryReceiver
    };

    nrf24_t m_nrf;
    uint8_t m_transceiverChannel = 0;
    // OperationQueue m_queue;

    void initTransceiver();
    void setTransceiverMode(const TransceiverMode mode, const std::string& txAddress = {});
    std::vector<protocol_msg_t> readIncomingMessages();

    InterruptResult checkInterrupt();
    bool isInterruptTriggered() const;
    InterruptResult waitForInterrupt();
    Result sendProtocolMsg(const std::string& address, const protocol_msg_t& msg);
    Result receiveProtocolMessages(std::vector<protocol_msg_t>& messages);
    Response sendRequest(const Request& request);
};

}
