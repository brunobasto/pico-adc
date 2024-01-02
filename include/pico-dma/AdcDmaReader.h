#ifndef ADC_DMA_READER_H
#define ADC_DMA_READER_H

#include <stdint.h>

class AdcDmaReader {
public:
    AdcDmaReader(int channel = 4, int depth = 10000);
    ~AdcDmaReader();

    void startCapture();
    void stopCapture();
    void registerCallback(void (*callback)(uint8_t* buffer, int size));

private:
    static void dmaHandlerA();
    static void dmaHandlerB();

    void setupAdc();
    void setupDma();
    int getGpioForAdcChannel(int channel);

    static uint dmaChanA, dmaChanB;
    static uint8_t* captureBufA;
    static uint8_t* captureBufB;
    static int captureDepth;
    static void (*userCallback)(uint8_t* buffer, int size);

    int captureChannel;
};

#endif // ADC_DMA_READER_H
