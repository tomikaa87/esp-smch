#pragma once

#include "Command.h"
#include "FixedSizeRingBuffer.h"
#include "Logger.h"
#include "nrf24.h"
#include "radio_protocol.h"
#include "Response.h"
#include "Request.h"

#include <queue>
#include <vector>

namespace radio
{

struct Message
{
    using Payload = std::vector<uint8_t>;

    Message() = default;
    Message(const Message&) = delete;
    Message(Message&&) = default;

    Message& operator=(const Message&) = delete;
    Message& operator=(Message&&) = default;

    Message(std::string address, Payload&& payload)
        : number(_nextNumber++)
        , address(std::move(address))
        , payload(std::move(payload))
    {}

    Message(const Request& request)
        : number(_nextNumber++)
        , address(request.address)
    {
        payload.resize(sizeof request.msg);
        memcpy(payload.data(), &request.msg, sizeof request.msg);
    }

    uint32_t number = 0;
    std::string address;
    Payload payload;

private:
    static uint32_t _nextNumber;
};

class Radio
{
public:
    Radio(uint8_t transceiverChannel);

    bool sendMessage(Message&& message);

    bool hasIncomingMessage() const;
    Message nextIncomingMessage();

    void task();

private:
    const Logger _log{ "Radio" };

    enum class TransceiverMode
    {
        PrimaryTransmitter,
        PrimaryReceiver
    };

    nrf24_t m_nrf;
    uint8_t m_transceiverChannel = 0;

    std::queue<Message, FixedSizeRingBuffer<Message, 5>> _transmitQueue;
    std::queue<Message, FixedSizeRingBuffer<Message, 5>> _receiveQueue;

    void initTransceiver();
    void setTransceiverMode(const TransceiverMode mode, const std::string& txAddress = {});

    bool isInterruptTriggered() const;

    static constexpr uint32_t MessageSendTimeoutMs = 1000;

    enum class State
    {
        Idle,
        SendNextQueuedMessage,
        WaitForMessageSent,
        ReadReceivedMessages
    } _state = State::Idle;
    uint32_t _timer = 0;

    void runStateMachine();
};

}
