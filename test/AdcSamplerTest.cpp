/***
 * Functional failure test
 */

#include "CppUTest/TestHarness.h"
#include "pico-adc/AdcSampler.h"
#include "pico-adc/IAdcHardware.h"

template <typename T>
class MockAdcHardware : public IAdcHardware<T>
{
public:
    bool initAdcCalled = false;
    // Mock methods
    void initAdc(const std::vector<int> &channels, int sampleDepth) override
    {
        // Simulate ADC initialization
        initAdcCalled = true;
    }

    void startSampling() override
    {
        // Simulate starting ADC sampling
    }

    void stopSampling() override
    {
        // Simulate stopping ADC samplingx
    }

    void registerCallback(void (*callback)(T *buffer, int size)) override
    {
        // Store the callback for later invocation in tests
    }

    // Add methods to manually trigger the callback, set internal states, etc.
};

TEST_GROUP(Init){};

TEST(Init, ShouldCallInitAdc)
{
    MockAdcHardware<uint8_t> mockHardware;
    AdcSampler<uint8_t> sampler(&mockHardware, {0}, 100);
    CHECK_EQUAL(mockHardware.initAdcCalled, true);
}