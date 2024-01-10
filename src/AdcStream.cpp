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
    adc_fifo_setup(
        true,                           // Enables the FIFO. When true, ADC readings are stored in the FIFO buffer.
        true,                           // If true, the FIFO generates DMA requests when it has data.
        1,                              // The number of conversions after which a DMA request is generated. Here, it's set to 1, meaning after each conversion.
        false,                          // Determines whether a DMA request is generated when the FIFO is full. Here, it's set to false.
        std::is_same<T, uint8_t>::value // Sets the data format in the FIFO. If true (for uint8_t), data is 8 bits; if false (for uint16_t), data is 12 bits.
    );

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
    // Claiming DMA channels
    dmaChanA = dma_claim_unused_channel(true); // Claims an unused DMA channel and assigns it to dmaChanA.
    dmaChanB = dma_claim_unused_channel(true); // Claims another unused DMA channel and assigns it to dmaChanB.

    // Setting up DMA channel A
    dma_channel_config dmaCfgA, dmaCfgB;

    dmaCfgA = dma_channel_get_default_config(dmaChanA);                // Gets the default DMA channel configuration for dmaChanA.
    channel_config_set_transfer_data_size(&dmaCfgA, getDmaDataSize()); // Sets the data size for transfers (8 or 16 bits, based on T).
    channel_config_set_read_increment(&dmaCfgA, false);                // Disables incrementing the read address (since reading from a constant address - the ADC FIFO).
    channel_config_set_write_increment(&dmaCfgA, true);                // Enables incrementing the write address (writing to different parts of the buffer).
    channel_config_set_dreq(&dmaCfgA, DREQ_ADC);                       // Sets the DMA to trigger on ADC conversion complete.
    channel_config_set_chain_to(&dmaCfgA, dmaChanB);                   // Chains dmaChanA to dmaChanB, so that when dmaChanA completes, dmaChanB starts.

    // Setting up DMA channel B
    dmaCfgB = dma_channel_get_default_config(dmaChanB);                // Gets the default DMA channel configuration for dmaChanB.
    channel_config_set_transfer_data_size(&dmaCfgB, getDmaDataSize()); // Same as above, sets the data size for transfers.
    channel_config_set_read_increment(&dmaCfgB, false);                // Same as above, disables incrementing the read address.
    channel_config_set_write_increment(&dmaCfgB, true);                // Same as above, enables incrementing the write address.
    channel_config_set_dreq(&dmaCfgB, DREQ_ADC);                       // Same as above, sets the DMA to trigger on ADC conversion complete.
    channel_config_set_chain_to(&dmaCfgB, dmaChanA);                   // Chains dmaChanB to dmaChanA, creating a loop between A and B.

    // Configuring the channels for continuous circular buffer operation
    dma_channel_configure(dmaChanA, &dmaCfgA, captureBufA, &adc_hw->fifo, captureDepth, true);  // Configures dmaChanA to write from ADC FIFO to captureBufA.
    dma_channel_configure(dmaChanB, &dmaCfgB, captureBufB, &adc_hw->fifo, captureDepth, false); // Configures dmaChanB to write from ADC FIFO to captureBufB.

    // Setting up IRQ (Interrupt Request) for DMA channels
    dma_channel_set_irq0_enabled(dmaChanA, true);      // Enables interrupt requests for dmaChanA.
    irq_set_exclusive_handler(DMA_IRQ_0, dmaHandlerA); // Sets dmaHandlerA as the interrupt handler for dmaChanA.
    irq_set_enabled(DMA_IRQ_0, true);                  // Enables IRQ 0.

    dma_channel_set_irq1_enabled(dmaChanB, true);      // Enables interrupt requests for dmaChanB.
    irq_set_exclusive_handler(DMA_IRQ_1, dmaHandlerB); // Sets dmaHandlerB as the interrupt handler for dmaChanB.
    irq_set_enabled(DMA_IRQ_1, true);                  // Enables IRQ 1.
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