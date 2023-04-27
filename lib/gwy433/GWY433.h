#ifndef ESP_GWY_433_H__
#define ESP_GWY_433_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <WiFiUdp.h>

class GWY433
{
public:
    struct Signal
    {
        uint8_t repeatCount;
        uint16_t pauseLength;
        uint16_t pulseLength;
        uint16_t numberOfSignalChanges;

        std::vector<uint8_t> multipliers;
    };

public:
    GWY433(uint8_t pin_tx);

    void begin(int port);
    void listen();

    /**
     * Expects a packet in the standard format of the GWY433.
     * This means, data should look like this:
     *
     * TXP:0,0,<repeat count>,<pause length>,<pulse length>,
     * <number of signal changes>,<pulse length multipliers (actual signal)...>;
     */
    void parseAndSend(const char *data, size_t lenBytes);

    void send(Signal const &signal);

private:
    uint8_t pin_tx;
    WiFiUDP mUdp;

    IPAddress ip;
};

#endif
