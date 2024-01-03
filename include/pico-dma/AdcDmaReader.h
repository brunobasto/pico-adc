#ifndef ADC_DMA_READER_H
#define ADC_DMA_READER_H

#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

class AdcDmaReader
{
public:
    AdcDmaReader(int channel = 4, int depth = 10000);
    ~AdcDmaReader();

    void startCapture();
    void stopCapture();
    void registerCallback(void (*callback)(uint8_t id, uint8_t *buffer, int size));
    uint8_t* getBufferA() const;
    uint8_t* getBufferB() const;

private:
    static void dmaHandlerA();
    static void dmaHandlerB();

    void setupAdc();
    void setupDma();

    static uint8_t captureBufA[1024];
    static uint8_t captureBufB[1024];
    static uint dmaChanA, dmaChanB;
    static int captureDepth;
    static void (*userCallback)(uint8_t id, uint8_t *buffer, int size);

    int captureChannel;
};

#endif // ADC_DMA_READER_H
