#pragma once

#include "Device.h"
#include "Logger.h"
#include "radio_protocol.h"

#include <bitset>
#include <cstdint>
#include <vector>

namespace radio
{
    class Message;
    class Radio;
}

class DeviceHub
{
public:
    static constexpr auto MaxDeviceCount = 10;
    static constexpr uint32_t ScanStatusRequestTimeoutMs = 2000;

    explicit DeviceHub(radio::Radio& radio);

    bool startScan();

    void task();

private:
    Logger _log{ "Hub" };
    radio::Radio& _radio;

    std::bitset<MaxDeviceCount> _activeBits;
    std::array<uint32_t, MaxDeviceCount> _lastSeenTimes;

    std::vector<protocol_msg_t> _receivedMessages;

    bool sendMessageToDevice(size_t index, const protocol_msg_t& message);

    enum class State {
        Idle,
        Scanning
    } _state = State::Idle;

    struct Scanner {
        enum class State {
            SendStatusRequest,
            WaitStatusResponse,
            Finish
        } state = State::SendStatusRequest;

        size_t currentIndex = 0;
        uint32_t timer = 0;
    } _scanner;

    bool isIdle() const { return _state == State::Idle; }

    void runStateMachine();
    bool runScanStateMachine();

    void receiveMessages();

    static protocol_msg_t toProtoMsg(const radio::Message& message);
    static bool isMessageValid(const protocol_msg_t& message);
    static protocol_msg_t createStatusRequestMessage();
};