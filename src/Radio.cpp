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

//#define ENABLE_DEBUG_LOG

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

bool Radio::sendCommand(Command command, const std::string& address)
{
    // auto task = std::make_shared<Task>(createCommandRequest(command, address));

    // m_queue.enqueue([this, task] {
    //     auto response = sendRequest(task->request);
    //     task->promise.set_value(response);
    // });

    // return task->promise.get_future();
    return true;
}

bool Radio::readStatus(const std::string& address)
{
    // auto task = std::make_shared<Task>(createStatusRequest(address));

    // m_queue.enqueue([this, task] {
    //     auto response = sendRequest(task->request);
    //     task->promise.set_value(response);
    // });

    // return task->promise.get_future();
    return true;
}

void Radio::task()
{

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

std::vector<protocol_msg_t> Radio::readIncomingMessages()
{
    _log.debug("data received");

    std::vector<protocol_msg_t> messages;

    while (1)
    {
        auto fifoStatus = nrf24_get_fifo_status(&m_nrf);

        if (fifoStatus.RX_EMPTY)
        {
            _log.debug("RX FIFO empty");
            break;
        }

        _log.debug("reading incoming payload");

        protocol_msg_t msg;

        nrf24_read_rx_payload(&m_nrf, msg.data, sizeof(protocol_msg_t));

        if (msg.msg_magic != PROTO_MSG_MAGIC)
        {
            _log.warning("incoming protocol message is invalid");
            break;
        }

        messages.push_back(std::move(msg));
    };

    return messages;
}

Radio::InterruptResult Radio::checkInterrupt()
{
    if (isInterruptTriggered())
    {
        auto status = nrf24_get_status(&m_nrf);

#if defined ENABLE_DEBUG_LOG && defined NRF24_ENABLE_DUMP_REGISTERS
        nrf24_dump_registers(&m_nrf);
#endif

        nrf24_clear_interrupts(&m_nrf);

        if (status.MAX_RT)
            return InterruptResult::PacketLost;

        if (status.RX_DR)
            return InterruptResult::DataReceived;

        if (status.TX_DS)
            return InterruptResult::DataSent;
    }

    return InterruptResult::Timeout;
}

bool Radio::isInterruptTriggered() const
{
    return digitalRead(Pins::NrfIrq) == 0;
}

Radio::InterruptResult Radio::waitForInterrupt()
{
    _log.debug("waiting for interrupt");

    auto startTime = std::chrono::steady_clock::now();
    auto result = InterruptResult::Timeout;

    while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(2))
    {
        if (isInterruptTriggered())
        {
            result = checkInterrupt();
            break;
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // qCDebug(RadioLog) << "interrupt result:" << result;

    return result;
}

Result Radio::sendProtocolMsg(const std::string& address, const protocol_msg_t& msg)
{
    _log.debug("sending protocol message to: %s, length: %u", address.c_str(), sizeof(protocol_msg_t));

#ifdef ENABLE_DEBUG_LOG
    print_protocol_message(&msg);
#endif

    setTransceiverMode(TransceiverMode::PrimaryTransmitter, address);

    nrf24_write_tx_payload(&m_nrf, msg.data, sizeof(protocol_msg_t));
    nrf24_power_up(&m_nrf);

    auto interruptResult = waitForInterrupt();

    if (interruptResult == InterruptResult::Timeout)
        return Result::Timeout;

    if (interruptResult == InterruptResult::PacketLost)
        return Result::PacketLost;

    // assert(interruptResult == InterruptResult::DataSent);

    return Result::Succeeded;
}

Result Radio::receiveProtocolMessages(std::vector<protocol_msg_t>& messages)
{
    _log.debug("receiving protocol messages");

    setTransceiverMode(TransceiverMode::PrimaryReceiver);

    nrf24_power_up(&m_nrf);

    auto interruptResult = waitForInterrupt();

    if (interruptResult == InterruptResult::Timeout)
        return Result::Timeout;

    // assert(interruptResult == InterruptResult::DataReceived);

    messages = readIncomingMessages();

    _log.debug("number of received messages: %d", messages.size());

    return Result::Succeeded;
}

Response Radio::sendRequest(const Request& request)
{
    Response response;

    response.result = sendProtocolMsg(request.address, request.msg);

    if (response.result != Result::Succeeded)
        return response;

    response.result = receiveProtocolMessages(response.messages);

    return response;
}

// QDebug& operator<<(QDebug& dbg, Radio::InterruptResult ir)
// {
//     switch (ir)
//     {
//     case Radio::InterruptResult::DataReceived:
//         dbg << "DataReceived";
//         break;

//     case Radio::InterruptResult::DataSent:
//         dbg << "DataSent";
//         break;

//     case Radio::InterruptResult::Timeout:
//         dbg << "Timeout";
//         break;

//     case Radio::InterruptResult::PacketLost:
//         dbg << "PacketLost";
//         break;
//     }

//     return dbg;
// }

}
