#pragma once

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include <vector>

#include <pico-adc/IAdcHardware.h>

template <typename T>
class PicoAdcHardware : public IAdcHardware<T>
{
private:
    std::vector<int> channels;
    int sampleDepth;
    uint dmaChanA;
    uint dmaChanB;
    void (*userCallback)(T *buffer, int size);

    static T *captureBufA;
    static T *captureBufB;
    static PicoAdcHardware<T> *instance;

    dma_channel_transfer_size getDmaDataSize()
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
            static_assert(std::is_same<T, void>::value, "Unsupported type for PicoAdcHardware");

            // This line is never reached but is required to avoid compile errors.
            return static_cast<dma_channel_transfer_size>(-1);
        }
    }

    static void dmaHandlerA()
    {
        if (instance && instance->userCallback)
            instance->userCallback(instance->captureBufA, instance->sampleDepth);
        dma_hw->ints0 = 1u << instance->dmaChanA;
        dma_channel_set_write_addr(instance->dmaChanA, instance->captureBufA, false);
    }

    static void dmaHandlerB()
    {
        if (instance && instance->userCallback)
            instance->userCallback(instance->captureBufB, instance->sampleDepth);
        dma_hw->ints1 = 1u << instance->dmaChanB;
        dma_channel_set_write_addr(instance->dmaChanB, instance->captureBufB, false);
    }

    void setupDma()
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
        dma_channel_configure(dmaChanA, &dmaCfgA, captureBufA, &adc_hw->fifo, sampleDepth, true);  // Configures dmaChanA to write from ADC FIFO to captureBufA.
        dma_channel_configure(dmaChanB, &dmaCfgB, captureBufB, &adc_hw->fifo, sampleDepth, false); // Configures dmaChanB to write from ADC FIFO to captureBufB.

        // Setting up IRQ (Interrupt Request) for DMA channels
        dma_channel_set_irq0_enabled(dmaChanA, true);      // Enables interrupt requests for dmaChanA.
        irq_set_exclusive_handler(DMA_IRQ_0, dmaHandlerA); // Sets dmaHandlerA as the interrupt handler for dmaChanA.
        irq_set_enabled(DMA_IRQ_0, true);                  // Enables IRQ 0.

        dma_channel_set_irq1_enabled(dmaChanB, true);      // Enables interrupt requests for dmaChanB.
        irq_set_exclusive_handler(DMA_IRQ_1, dmaHandlerB); // Sets dmaHandlerB as the interrupt handler for dmaChanB.
        irq_set_enabled(DMA_IRQ_1, true);                  // Enables IRQ 1.
    }

public:
    PicoAdcHardware()
    {
        instance = this;
    }

    ~PicoAdcHardware()
    {
        if (instance == this)
            instance = nullptr;
    }

    void initAdc(const std::vector<int> &channels, int sampleDepth) override
    {
        this->channels = channels;
        this->sampleDepth = sampleDepth;

        adc_init();
        // Configure ADC for round-robin on specified channels
        uint mask = 0;
        for (int channel : channels)
        {
            mask |= (1u << channel);
        }
        adc_set_round_robin(mask);
        adc_fifo_setup(true, true, 1, false, true);
        adc_set_clkdiv(0);

        // Setup DMA channels and other ADC configurations
        setupDma();

        captureBufA = new T[sampleDepth];
        captureBufB = new T[sampleDepth];
    }

    void startSampling() override
    {
        // Start DMA and ADC
        dma_channel_start(dmaChanA);
        adc_run(true);
    }

    void stopSampling() override
    {
        // Stop DMA and ADC
        adc_run(false);
        dma_channel_abort(dmaChanA);
        dma_channel_abort(dmaChanB);
    }

    void registerCallback(void (*callback)(T *buffer, int size))
    {
        userCallback = callback;
    }
};

template <typename T>
PicoAdcHardware<T> *PicoAdcHardware<T>::instance = nullptr;

template <typename T>
T *PicoAdcHardware<T>::captureBufA = nullptr;

template <typename T>
T *PicoAdcHardware<T>::captureBufB = nullptr;

template class PicoAdcHardware<uint8_t>;
template class PicoAdcHardware<uint16_t>;