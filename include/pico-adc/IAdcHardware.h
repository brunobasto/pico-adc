#pragma once

#include <vector>

template <typename T>
class IAdcHardware
{
public:
    virtual void initAdc(const std::vector<int> &channels, int sampleDepth) = 0;
    virtual void startSampling() = 0;
    virtual void stopSampling() = 0;
    virtual void registerCallback(void (*callback)(T *buffer, int size)) = 0;
};
