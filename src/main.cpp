#include <hardware/gpio.h>
#include "hardware/adc.h"
#include <pico/stdlib.h>
#include <stdio.h>

#include "pico-adc/AdcSampler.h"
#include "pico-adc/PicoAdcHardware.h"

#define CAPTURE_DEPTH 1000
#define CAPTURE_CHANNEL 1

static bool canPrint = true;
uint8_t* buffer = nullptr;

void adc_capture(uint8_t *buf, int count)
{
    canPrint = true;
    buffer = buf;
}

// Usage example
int main()
{
    stdio_init_all();

    std::vector<int> channels = {0}; // Example channel configuration

    // Create a hardware-specific instance
    PicoAdcHardware<uint8_t> picoHardware;
    AdcSampler<uint8_t> sampler(&picoHardware, channels, CAPTURE_DEPTH);

    sampler.onSamplingComplete(adc_capture);
    sampler.startCapture();

    // Main loop
    while (true)
    {
        if (canPrint)
        {
            printf("Buffer: ");
            for (int i = 0; i < CAPTURE_DEPTH; i++)
            {
                printf("%d ", buffer[i]);
            }
            printf("\n");

            canPrint = false;
        }
    }

    return 0;
}
