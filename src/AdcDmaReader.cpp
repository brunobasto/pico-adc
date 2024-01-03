#include "pico-dma/AdcDmaReader.h"

AdcDmaReader::AdcDmaReader(int channel, int depth) : captureChannel(channel) {
    captureDepth = depth;
    AdcDmaReader::captureBufA = new uint8_t[captureDepth];
    AdcDmaReader::captureBufB = new uint8_t[captureDepth];
    setupAdc();
    setupDma();
};

AdcDmaReader::~AdcDmaReader() {
    delete[] captureBufA;
    delete[] captureBufB;
};

void AdcDmaReader::startCapture() {
    adc_run(true);
};

void AdcDmaReader::stopCapture() {
    adc_run(false);
};

uint8_t* AdcDmaReader::getBufferA() const {
    return captureBufA;
};

uint8_t* AdcDmaReader::getBufferB() const {
    return captureBufB;
};

void AdcDmaReader::registerCallback(void (*callback)(uint8_t id, uint8_t* buffer, int size)) {
    userCallback = callback;
};

void AdcDmaReader::dmaHandlerA() {
    if (userCallback) userCallback(0, captureBufA, captureDepth);
    dma_hw->ints0 = 1u << dmaChanA;
    dma_channel_set_write_addr(dmaChanA, captureBufA, false);
};

void AdcDmaReader::dmaHandlerB() {
    if (userCallback) userCallback(1, captureBufB, captureDepth);
    dma_hw->ints1 = 1u << dmaChanB;
    dma_channel_set_write_addr(dmaChanB, captureBufB, false);
};

void AdcDmaReader::setupAdc() {
    adc_gpio_init(26 + captureChannel);
    adc_init();
    adc_select_input(captureChannel);
    adc_fifo_setup(true, true, 1, false, true);
    adc_set_clkdiv(0);
};

void AdcDmaReader::setupDma() {
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
};

// Static member initialization
uint AdcDmaReader::dmaChanA = 0;
uint AdcDmaReader::dmaChanB = 0;
uint8_t* AdcDmaReader::captureBufA = nullptr;
uint8_t* AdcDmaReader::captureBufB = nullptr;
int AdcDmaReader::captureDepth = 0;
void (*AdcDmaReader::userCallback)(uint8_t id, uint8_t* buffer, int size) = nullptr;
