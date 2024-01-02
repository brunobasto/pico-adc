#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

class AdcDmaReader {
public:
    AdcDmaReader(int channel = 4, int depth = 10000) : captureChannel(channel) {
        captureDepth = depth;
        captureBufA = new uint8_t[captureDepth];
        captureBufB = new uint8_t[captureDepth];
        setupAdc();
        setupDma();
    }

    ~AdcDmaReader() {
        delete[] captureBufA;
        delete[] captureBufB;
    }

    void startCapture() {
        adc_run(true);
    }

    void stopCapture() {
        adc_run(false);
    }

    void registerCallback(void (*callback)(uint8_t* buffer, int size)) {
        userCallback = callback;
    }

private:
    static void dmaHandlerA() {
        if (userCallback) userCallback(captureBufA, captureDepth);
        dma_hw->ints0 = 1u << dmaChanA;
        dma_channel_set_write_addr(dmaChanA, captureBufA, false);
    }

    static void dmaHandlerB() {
        if (userCallback) userCallback(captureBufB, captureDepth);
        dma_hw->ints1 = 1u << dmaChanB;
        dma_channel_set_write_addr(dmaChanB, captureBufB, false);
    }

    void setupAdc() {
        adc_gpio_init(26 + captureChannel);
        adc_init();
        adc_select_input(captureChannel);
        adc_fifo_setup(true, true, 1, false, true);
        adc_set_clkdiv(0);
    }

    void setupDma() {
        dma_channel_config dmaCfgA, dmaCfgB;

        dmaChanA = dma_claim_unused_channel(true);
        dmaChanB = dma_claim_unused_channel(true);

        dmaCfgA = dma_channel_get_default_config(dmaChanA);
        channel_config_set_transfer_data_size(&dmaCfgA, DMA_SIZE_8);
        channel_config_set_read_increment(&dmaCfgA, false);
        channel_config_set_write_increment(&dmaCfgA, true);
        channel_config_set_dreq(&dmaCfgA, DREQ_ADC);
        channel_config_set_chain_to(&dmaCfgA, dmaChanB);

        dmaCfgB = dma_channel_get_default_config(dmaChanB);
        channel_config_set_transfer_data_size(&dmaCfgB, DMA_SIZE_8);
        channel_config_set_read_increment(&dmaCfgB, false);
        channel_config_set_write_increment(&dmaCfgB, true);
        channel_config_set_dreq(&dmaCfgB, DREQ_ADC);
        channel_config_set_chain_to(&dmaCfgB, dmaChanA);

        dma_channel_configure(dmaChanA, &dmaCfgA, captureBufA, &adc_hw->fifo, captureDepth, true);
        dma_channel_configure(dmaChanB, &dmaCfgB, captureBufB, &adc_hw->fifo, captureDepth, false);

        dma_channel_set_irq0_enabled(dmaChanA, true);
        irq_set_exclusive_handler(DMA_IRQ_0, dmaHandlerA);
        irq_set_enabled(DMA_IRQ_0, true);

        dma_channel_set_irq1_enabled(dmaChanB, true);
        irq_set_exclusive_handler(DMA_IRQ_1, dmaHandlerB);
        irq_set_enabled(DMA_IRQ_1, true);
    }

    static uint dmaChanA, dmaChanB;
    static uint8_t* captureBufA;
    static uint8_t* captureBufB;
    static int captureDepth;
    static void (*userCallback)(uint8_t* buffer, int size);

    int captureChannel;
};

// Static member initialization
uint AdcDmaReader::dmaChanA = 0;
uint AdcDmaReader::dmaChanB = 0;
uint8_t* AdcDmaReader::captureBufA = nullptr;
uint8_t* AdcDmaReader::captureBufB = nullptr;
int AdcDmaReader::captureDepth = 0;
void (*AdcDmaReader::userCallback)(uint8_t* buffer, int size) = nullptr;
