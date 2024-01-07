#include <hardware/gpio.h>
#include "hardware/adc.h"
#include <pico-adc/AdcStream.h>
#include <pico/stdlib.h>
#include <stdio.h>

#define CAPTURE_DEPTH 1000
#define CAPTURE_CHANNEL 1

static bool canPrint = true;
static uint8_t readyId = 0;

void __not_in_flash_func(adc_capture)(uint8_t id, uint8_t *buf, int count)
{
    canPrint = true;
    readyId = id;
}

// Usage example
int main()
{
    stdio_init_all();
    gpio_set_dir_all_bits(0);
    adc_set_temp_sensor_enabled(true);

    std::vector<int> channels = {0, 1}; // Example channels
    AdcStream<uint8_t> adcStream(channels, CAPTURE_DEPTH);
    adcStream.registerCallback(adc_capture);
    adcStream.startCapture();

    // Main loop
    while (true)
    {
        if (canPrint)
        {
            if (readyId == 0)
            {
                printf("Buffer A: ");
                for (int i = 0; i < CAPTURE_DEPTH; i++)
                {
                    printf("%d ", adcStream.getBufferA()[i]);
                }
                printf("\n");
            }
            else
            {

                printf("Buffer B: ");
                for (int i = 0; i < CAPTURE_DEPTH; i++)
                {
                    printf("%d ", adcStream.getBufferB()[i]);
                }
                printf("\n");
            }

            canPrint = false;
        }
    }

    return 0;
}
