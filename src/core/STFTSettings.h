#pragma once

enum class WindowType
{
    rectangular,
    hann,
    hamming,
    blackman,
    blackmanHarris
};

struct STFTSettings
{
    WindowType windowType = WindowType::hann;
    int fftSize = 2048;
    int overlapPercent = 50;
};