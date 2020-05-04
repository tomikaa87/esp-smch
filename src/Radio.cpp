/*
 * Radio.cpp
 *
 *  Created on: 2018. maj. 26.
 *      Author: tomikaa
 */

#include "Radio.h"
#include "Utils.h"
// #include "Task.h"

#include <chrono>
#include <cstdio>

#include <Arduino.h>
#include <SPI.h>

#include <future>

namespace radio
{

uint32_t Message::_nextNumber = 0;

// #define ENABLE_DEBUG_LOG

static const uint8_t ReceiveAddress[] = { 'S', 'M', 'R', 'H', '1' };

namespace Pins
{
    static constexpr auto NrfCe = D1;
    static constexpr auto NrfCsn = D8;
    static constexpr auto NrfIrq = D3;
}

Radio::Radio(uint8_t transceiverChannel)
    : m_transceiverChannel(transceiverChannel)
{
    _log.info("initializing");

    initTransceiver();
}

bool Radio::sendMessage(Message&& message)
{
    _log.debug("sending message (enqueue): number=%lu, address=%s, length=%lu",
        message.number, message.address.c_str(), message.payload.size());

    _transmitQueue.push(std::move(message));

    _log.debug("pending messages in transmit queue: %lu", _transmitQueue.size());

    return true;
}

bool Radio::hasIncomingMessage() const
{
    return !_receiveQueue.empty();
}

Message Radio::nextIncomingMessage()
{
    auto message = std::move(_receiveQueue.front());
    _receiveQueue.pop();

    _log.debug("dequeued message: number=%lu, address=%s, length=%lu",
        message.number, message.address.c_str(), message.payload.size());

    return message;
}

void Radio::task()
{
    runStateMachine();
}

void Radio::initTransceiver()
{
    _log.debug("initializing transceiver, channel: %u", m_transceiverChannel);

    pinMode(Pins::NrfCe, OUTPUT);
    pinMode(Pins::NrfCsn, OUTPUT);
    pinMode(Pins::NrfIrq, INPUT);

    digitalWrite(Pins::NrfCe, HIGH);
    digitalWrite(Pins::NrfCsn, HIGH);

    nrf24_init(
        &m_nrf,
        // set_ce
        [] (const nrf24_state_t state) {
            digitalWrite(Pins::NrfCe, state == NRF24_HIGH ? HIGH : LOW);
        },
        // set_csn
        [] (const nrf24_state_t state) {
            digitalWrite(Pins::NrfCsn, state == NRF24_HIGH ? HIGH : LOW);
        },
        // spi_exchange
        [] (const uint8_t data) {
            return SPI.transfer(data);
        }
    );

    nrf24_rf_setup_t rf_setup;
    memset(&rf_setup, 0, sizeof (nrf24_rf_setup_t));
    rf_setup.P_RF_DR_HIGH = 0;
    rf_setup.P_RF_DR_LOW = 1;
    rf_setup.P_RF_PWR = NRF24_RF_OUT_PWR_0DBM;
    nrf24_set_rf_setup(&m_nrf, rf_setup);

    nrf24_set_rf_channel(&m_nrf, m_transceiverChannel);

    nrf24_setup_retr_t setup_retr;
    setup_retr.ARC = 15;
    setup_retr.ARD = 15;
    nrf24_set_setup_retr(&m_nrf, setup_retr);

    nrf24_set_rx_payload_len(&m_nrf, 0, NRF24_DEFAULT_PAYLOAD_LEN);

    nrf24_clear_interrupts(&m_nrf);

#if defined ENABLE_DEBUG_LOG && defined NRF24_ENABLE_DUMP_REGISTERS
    nrf24_dump_registers(&m_nrf);
#endif
}

void Radio::setTransceiverMode(const TransceiverMode mode, const std::string& txAddress)
{
    _log.debug("setting mode to %s", mode == TransceiverMode::PrimaryReceiver ? "PRX" : "PTX");

    if (!txAddress.empty())
        _log.debug("transmit address: %s, length: %d", txAddress.c_str(),  txAddress.size());

    nrf24_power_down(&m_nrf);

    nrf24_clear_interrupts(&m_nrf);

    // Reset packet loss counter PLOS_CNT
    nrf24_set_rf_channel(&m_nrf, m_transceiverChannel);

    nrf24_flush_rx(&m_nrf);
    nrf24_flush_tx(&m_nrf);

    nrf24_config_t config;
    memset(&config, 0, sizeof (nrf24_config_t));
    config.EN_CRC = 1;
    config.PWR_UP = 1;

    if (mode == TransceiverMode::PrimaryReceiver)
    {
        nrf24_set_rx_address(&m_nrf, 0, ReceiveAddress, sizeof(ReceiveAddress));
        config.PRIM_RX = 1;
    }
    else
    {
        nrf24_set_tx_address(&m_nrf, reinterpret_cast<const uint8_t*>(txAddress.c_str()), txAddress.size() & 0x07);
        nrf24_set_rx_address(&m_nrf, 0, reinterpret_cast<const uint8_t*>(txAddress.c_str()), txAddress.size() & 0x07);
    }

    nrf24_set_config(&m_nrf, config);

#if defined ENABLE_DEBUG_LOG && defined NRF24_ENABLE_DUMP_REGISTERS
    nrf24_dump_registers(&m_nrf);
#endif
}

bool Radio::isInterruptTriggered() const
{
    return digitalRead(Pins::NrfIrq) == 0;
}

void Radio::runStateMachine()
{
    switch (_state) {
        case State::Idle: {
            if (isInterruptTriggered()) {
                const auto status = nrf24_get_status(&m_nrf);

                if (status.RX_DR) {
                    _log.debug("message received");
                    _state = State::ReadReceivedMessages;
                    break;
                }
            }

            if (!_transmitQueue.empty()) {
                _log.debug("sending next message from transmit queue");
                _state = State::SendNextQueuedMessage;
            }

            break;
        }

        case State::SendNextQueuedMessage: {
            const auto message = std::move(_transmitQueue.front());
            _transmitQueue.pop();
            _log.debug("remaining messages in transmit queue: %lu", _transmitQueue.size());

            setTransceiverMode(TransceiverMode::PrimaryTransmitter, message.address);
            nrf24_write_tx_payload(&m_nrf, message.payload.data(), message.payload.size());
            nrf24_power_up(&m_nrf);

            _state = State::WaitForMessageSent;
            _timer = millis();

            break;
        }

        case State::WaitForMessageSent: {
            if (!isInterruptTriggered()) {
                if (millis() - _timer >= MessageSendTimeoutMs) {
                    _log.error("message sending timed out");

                    setTransceiverMode(TransceiverMode::PrimaryReceiver);
                    nrf24_power_up(&m_nrf);

                    _state = State::Idle;

                    break;
                }

                const auto status = nrf24_get_status(&m_nrf);

                if (status.TX_DS) {
                    _log.debug("message sent");
                } else if (status.MAX_RT) {
                    _log.debug("maximum transmit retry count reached");
                }

                setTransceiverMode(TransceiverMode::PrimaryReceiver);
                nrf24_power_up(&m_nrf);

                _state = State::Idle;
            }

            break;
        }

        case State::ReadReceivedMessages: {
            _log.debug("reading received messages");

            nrf24_power_down(&m_nrf);
            nrf24_clear_interrupts(&m_nrf);

            while (!nrf24_get_fifo_status(&m_nrf).RX_EMPTY) {
                Message::Payload payload;
                payload.resize(NRF24_DEFAULT_PAYLOAD_LEN);
                nrf24_read_rx_payload(&m_nrf, payload.data(), payload.size());

                std::vector<uint8_t> address;
                address.resize(5);
                nrf24_get_rx_address(&m_nrf, 0, address.data(), address.size());

                Message message{
                    std::string{ reinterpret_cast<const char*>(address.data()), address.size() },
                    std::move(payload)
                };

                _receiveQueue.push(std::move(message));
            }

            nrf24_power_up(&m_nrf);

            _log.debug("pending received messages: %lu", _receiveQueue.size());

            _state = State::Idle;

            break;
        }
    }
}

}