#include "pico-adc/AdcStream.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/irq.h"
#include <stdio.h>

template <typename T>
AdcStream<T>::AdcStream(const std::vector<int> &channels, int depth) : adcChannels(channels)
{
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value,
                  "AdcStream can only be instantiated with uint8_t or uint16_t");

    captureDepth = depth;
    captureBufA = new T[captureDepth];
    captureBufB = new T[captureDepth];
    setupAdc();
    setupDma();
};

template <typename T>
AdcStream<T>::~AdcStream()
{
    delete[] captureBufA;
    delete[] captureBufB;
};

template <typename T>
void AdcStream<T>::startCapture()
{
    adc_run(true);
};

template <typename T>
void AdcStream<T>::stopCapture()
{
    adc_run(false);
};

template <typename T>
T *AdcStream<T>::getBufferA() const
{
    return captureBufA;
};

template <typename T>
T *AdcStream<T>::getBufferB() const
{
    return captureBufB;
};

template <typename T>
void AdcStream<T>::registerCallback(void (*callback)(uint8_t id, T *buffer, int size))
{
    userCallback = callback;
};

template <typename T>
void AdcStream<T>::dmaHandlerA()
{
    if (userCallback)
        userCallback(0, captureBufA, captureDepth);
    dma_hw->ints0 = 1u << dmaChanA;
    dma_channel_set_write_addr(dmaChanA, captureBufA, false);
};

template <typename T>
void AdcStream<T>::dmaHandlerB()
{
    if (userCallback)
        userCallback(1, captureBufB, captureDepth);
    dma_hw->ints1 = 1u << dmaChanB;
    dma_channel_set_write_addr(dmaChanB, captureBufB, false);
};

template <typename T>
void AdcStream<T>::setupAdc()
{
    adc_init();
    configureRoundRobin();
    adc_fifo_setup(true, true, 1, false, std::is_same<T, uint8_t>::value);
    adc_set_clkdiv(0);
};

template <typename T>
void AdcStream<T>::configureRoundRobin()
{
    uint mask = 0;
    for (int channel : adcChannels)
    {
        mask |= (1u << channel);
    }
    adc_set_round_robin(mask);
}

template <typename T>
dma_channel_transfer_size AdcStream<T>::getDmaDataSize()
{
    if constexpr (std::is_same<T, uint8_t>::value)
    {
        return DMA_SIZE_8;
    }
    else if constexpr (std::is_same<T, uint16_t>::value)
    {
        return DMA_SIZE_16;
    }
    else
    {
        static_assert(std::is_same<T, void>::value, "Unsupported type for AdcStream");
        return -1; // This line is never reached but is required to avoid compile errors.
    }
}

template <typename T>
void AdcStream<T>::setupDma()
{
    dma_channel_config dmaCfgA, dmaCfgB;

    dmaChanA = dma_claim_unused_channel(true);
    dmaChanB = dma_claim_unused_channel(true);

    dmaCfgA = dma_channel_get_default_config(dmaChanA);
    channel_config_set_transfer_data_size(&dmaCfgA, getDmaDataSize());
    channel_config_set_read_increment(&dmaCfgA, false);
    channel_config_set_write_increment(&dmaCfgA, true);
    channel_config_set_dreq(&dmaCfgA, DREQ_ADC);
    channel_config_set_chain_to(&dmaCfgA, dmaChanB);

    dmaCfgB = dma_channel_get_default_config(dmaChanB);
    channel_config_set_transfer_data_size(&dmaCfgB, getDmaDataSize());
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

// Static member initialization
template <typename T>
T *AdcStream<T>::captureBufA = nullptr;

template <typename T>
T *AdcStream<T>::captureBufB = nullptr;

template <typename T>
uint AdcStream<T>::dmaChanA = 0;

template <typename T>
uint AdcStream<T>::dmaChanB = 0;

template <typename T>
int AdcStream<T>::captureDepth = 0;

template <typename T>
void (*AdcStream<T>::userCallback)(uint8_t id, T *buffer, int size) = nullptr;

template class AdcStream<uint8_t>;
template class AdcStream<uint16_t>;