#pragma once

#include <vector>
#include "pico-adc/IAdcHardware.h"

template <typename T>
class AdcSampler
{
private:
    IAdcHardware<T> *hardwareInterface;

public:
    AdcSampler(IAdcHardware<T> *hardware, const std::vector<int> &channels, int depth);

    void onSamplingComplete(void (*callback)(T *, int));
    void startCapture();
    void stopCapture();
};
