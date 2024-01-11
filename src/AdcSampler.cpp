#include <vector>
#include <stdint.h>
#include "pico-adc/IAdcHardware.h"

template <typename T>
class AdcSampler
{
private:
    IAdcHardware<T> *hardwareInterface;

public:
    AdcSampler(IAdcHardware<T> *hardware, const std::vector<int> &channels, int depth)
        : hardwareInterface(hardware)
    {
        hardwareInterface->initAdc(channels, depth);
    }

    void onSamplingComplete(void (*callback)(T *, int))
    {
        hardwareInterface->registerCallback(callback);
    }

    void startCapture()
    {
        hardwareInterface->startSampling();
    }

    void stopCapture()
    {
        hardwareInterface->stopSampling();
    }
};

template class AdcSampler<uint8_t>;
template class AdcSampler<uint16_t>;
