#include "DeviceHub.h"
#include "Radio.h"

#include <Arduino.h>

#include <algorithm>

DeviceHub::DeviceHub(radio::Radio& radio)
    : _radio(radio)
{}

bool DeviceHub::startScan()
{
    if (!isIdle()) {
        _log.warning_P(PSTR("cannot start scan, state machine is busy"));
        return false;
    }

    _log.info_P(PSTR("starting scan"));
    _state = State::Scanning;
    _scanner = {};
    return true;
}

void DeviceHub::task()
{
    receiveMessages();
    runStateMachine();
}

bool DeviceHub::sendMessageToDevice(const size_t index, const protocol_msg_t& message)
{
    char address[] = "SMRR0";
    address[4] = '0' + _scanner.currentIndex;
    _log.debug_P(PSTR("scan: target device address: %s"), address);

    std::vector<uint8_t> payload;
    payload.resize(sizeof(protocol_msg_t));
    memcpy(payload.data(), message.data, payload.size());

    radio::Message radioMessage{ address, std::move(payload) };
    return _radio.sendMessage(std::move(radioMessage));
}

void DeviceHub::runStateMachine()
{
    switch (_state) {
        case State::Idle: {
            break;
        }

        case State::Scanning: {
            if (runScanStateMachine()) {
                _state = State::Idle;
            }

            break;
        }
    }
}

bool DeviceHub::runScanStateMachine()
{
    switch (_scanner.state) {
        case Scanner::State::SendStatusRequest: {
            _scanner.timer = millis();
            _log.debug_P(PSTR("scan: sending status request to device index: %u"), _scanner.currentIndex);

            const auto request = createStatusRequestMessage();

            if (!sendMessageToDevice(_scanner.currentIndex, request)) {
                _log.warning_P(PSTR("scan: failed to send message"));
                _scanner.state = Scanner::State::Finish;
            }

            _scanner.state = Scanner::State::WaitStatusResponse;

            break;
        }

        case Scanner::State::WaitStatusResponse: {
            bool responseFound = false;

            for (const auto& msg : _receivedMessages) {
                _log.debug_P(PSTR("scan: reading received message"));

                if (msg.msg_type != PROTO_MSG_READ_STATUS_RESULT) {
                    _log.debug_P(PSTR("scan: dropping irrelevant message with type: %d"), msg.msg_type);
                    continue;
                }

                _log.debug_P(PSTR("scan: relevant response message found"));
                responseFound = true;

                break;
            }

            _receivedMessages.clear();

            if (responseFound) {
                _log.debug_P(PSTR("scan: found active device index: %u"), _scanner.currentIndex);
                _activeBits[_scanner.currentIndex] = true;
            } else if (millis() - _scanner.timer >= ScanStatusRequestTimeoutMs) {
                _log.debug_P(PSTR("scan: requiest timed out for device index: %u"), _scanner.currentIndex);
            } else {
                break;
            }

            if (++_scanner.currentIndex == MaxDeviceCount) {
                _scanner.state = Scanner::State::Finish;
            } else {
                _scanner.state = Scanner::State::SendStatusRequest;
            }

            break;
        }

        case Scanner::State::Finish: {
            _log.info("scan: finished, found %u active device(s)",
                _activeBits.count());

            return true;
        }
    }

    return false;
}

void DeviceHub::receiveMessages()
{
    while (_radio.hasIncomingMessage()) {
        const auto message = _radio.nextIncomingMessage();
        const auto protoMsg = toProtoMsg(message);

        if (isMessageValid(protoMsg)) {
            _log.warning_P(PSTR("dropping invalid message"));
            continue;
        }

        _log.debug_P(PSTR("received valid message"));
        print_protocol_message(&protoMsg);

        _receivedMessages.push_back(std::move(protoMsg));
    }
}

protocol_msg_t DeviceHub::toProtoMsg(const radio::Message& message)
{
    protocol_msg_t protoMsg;
    memcpy(&protoMsg, message.payload.data(), std::min(static_cast<size_t>(sizeof(protoMsg)), message.payload.size()));
    return protoMsg;
}

bool DeviceHub::isMessageValid(const protocol_msg_t& message)
{
    return message.msg_magic != PROTO_MSG_MAGIC;
}

protocol_msg_t DeviceHub::createStatusRequestMessage()
{
    protocol_msg_t m;
    protocol_msg_init(&m);
    m.msg_type = PROTO_MSG_READ_STATUS;
    return m;
}