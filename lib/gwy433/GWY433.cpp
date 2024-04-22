#include "GWY433.h"
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <Arduino.h>

GWY433::GWY433(uint8_t pin_tx)
    : pin_tx(pin_tx)
{
    pinMode(pin_tx, OUTPUT);
    digitalWrite(pin_tx, LOW);
}

void GWY433::begin(int port)
{
    mUdp.begin(port);
    this->ip = ip;
}

void GWY433::listen()
{
    uint8_t buffer[255] = {0};

    mUdp.parsePacket();
    if (mUdp.read(buffer, 255) > 0)
    {
        std::string strData(reinterpret_cast<char const *>(buffer), 255);
        // Check the content of the packet
        std::string begin;

        // Begins with "SEARCH HCGW"?
        begin = "SEARCH HCGW";
        if (strData.rfind(begin, 0) == 0)
        {
            // Reply with identification packet
            std::stringstream replyStr;
            replyStr << "HCGW:VC:Brennenstuhl;MC:0290217;FW:V016;IP:"
                     << (this->ip).toString().c_str()
                     << ";;";
            std::string strData = replyStr.str();
            mUdp.beginPacket(mUdp.remoteIP(), mUdp.remotePort());
            mUdp.write((const uint8_t *)strData.c_str(), strData.size());
            mUdp.endPacket();
            return;
        }

        // Begins with TXP?
        begin = "TXP:";
        if (strData.rfind(begin, 0) == 0)
        {
            Serial.println("Received packet");
            // Execute send operation
            parseAndSend(strData.c_str(), strData.size());
        }
    }
}

void GWY433::parseAndSend(const char *data, size_t lenBytes)
{
    if (lenBytes <= 0)
    {
        return;
    }
    // Cast binary to string
    std::string strData(reinterpret_cast<char const *>(data), lenBytes);

    // Check if string begins with TXP:
    std::string begin("TXP:0,0,");
    if (strData.rfind(begin, 0) != 0)
    {
        return;
    }

    Signal newSignal;

    // Create a stringstream from the data
    std::stringstream ss(strData);

    ss.seekg(begin.length()); // Ignore beginning
    std::string substrTmp;
    std::getline(ss, substrTmp, ',');
    newSignal.repeatCount = (uint8_t)std::atoi(substrTmp.c_str()); // Get repeatCount
    std::getline(ss, substrTmp, ',');
    newSignal.pauseLength = (uint16_t)std::atoi(substrTmp.c_str()); // Get pauseLength
    std::getline(ss, substrTmp, ',');
    newSignal.pulseLength = (uint16_t)std::atoi(substrTmp.c_str()); // Get pulseLength
    std::getline(ss, substrTmp, ',');
    newSignal.numberOfSignalChanges = (uint16_t)std::atoi(substrTmp.c_str()); // Get numberOfSignalChanges

    // Get the signal changes (pulse length multipliers)
    // and generate durations
    while (ss.good())
    {
        std::string tmp;
        std::getline(ss, tmp, ',');
        uint8_t mul = (uint8_t)std::atoi(tmp.c_str());
        newSignal.multipliers.push_back(mul);
    }

    // Send the signal
    send(newSignal);
}

void GWY433::send(Signal const &signal)
{
    uint16_t pulseLen = signal.pulseLength;

    // Retry n times
    for (uint8_t i = signal.repeatCount; i > 0; --i)
    {
        bool state = HIGH;
        for (auto const &value : signal.multipliers)
        {
            digitalWrite(pin_tx, state);
            delayMicroseconds((uint16_t)value * pulseLen);
            state = ((state == LOW) ? HIGH : LOW);
        }
        // Delay the pause at low state
        digitalWrite(pin_tx, LOW);
        delayMicroseconds(signal.pauseLength);
    }
}
