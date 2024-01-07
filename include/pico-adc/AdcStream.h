#ifndef ADC_STREAM_H
#define ADC_STREAM_H

#include <stdint.h>
#include <vector>
#include "pico/stdlib.h"
#include "hardware/dma.h"

template <typename T>
class AdcStream
{
public:
    AdcStream(const std::vector<int> &channels, int depth = 10000);
    ~AdcStream();

    void startCapture();
    void stopCapture();
    void registerCallback(void (*callback)(uint8_t id, T *buffer, int size));
    T *getBufferA() const;
    T *getBufferB() const;

private:
    static void dmaHandlerA();
    static void dmaHandlerB();

    void setupAdc();
    void setupDma();
    void configureRoundRobin();
    dma_channel_transfer_size getDmaDataSize();

    static T *captureBufA;
    static T *captureBufB;
    static uint dmaChanA, dmaChanB;
    static int captureDepth;
    static void (*userCallback)(uint8_t id, T *buffer, int size);

    std::vector<int> adcChannels;
    int captureChannel;
};

#endif // ADC_STREAM_H
