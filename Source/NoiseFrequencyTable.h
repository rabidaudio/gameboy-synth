#pragma once

#include <cstdint>

struct NoiseFrequencyParams
{
    uint8_t shift;
    uint8_t div;
    NoiseFrequencyParams(uint8_t shift, uint8_t div): shift(shift), div(div) {}
};

static const NoiseFrequencyParams NOISE_PARAM_TABLE[] =
{
    NoiseFrequencyParams(15, 7), // 1.14285714285714
    NoiseFrequencyParams(15, 6), // 1.33333333333333
    NoiseFrequencyParams(15, 5), // 1.6
    NoiseFrequencyParams(15, 4), // 2
    NoiseFrequencyParams(14, 7), // 2.28571428571429
    NoiseFrequencyParams(14, 6), // 2.66666666666667
    NoiseFrequencyParams(14, 5), // 3.2
    NoiseFrequencyParams(14, 4), // 4
    NoiseFrequencyParams(13, 7), // 4.57142857142857
    NoiseFrequencyParams(13, 6), // 5.33333333333333
    NoiseFrequencyParams(13, 5), // 6.4
    NoiseFrequencyParams(13, 4), // 8
    NoiseFrequencyParams(12, 7), // 9.14285714285714
    NoiseFrequencyParams(12, 6), // 10.6666666666667
    NoiseFrequencyParams(12, 5), // 12.8
    NoiseFrequencyParams(12, 4), // 16
    NoiseFrequencyParams(11, 7), // 18.2857142857143
    NoiseFrequencyParams(11, 6), // 21.3333333333333
    NoiseFrequencyParams(11, 5), // 25.6
    NoiseFrequencyParams(11, 4), // 32
    NoiseFrequencyParams(10, 7), // 36.5714285714286
    NoiseFrequencyParams(10, 6), // 42.6666666666667
    NoiseFrequencyParams(10, 5), // 51.2
    NoiseFrequencyParams(10, 4), // 64
    NoiseFrequencyParams(9, 7), // 73.1428571428571
    NoiseFrequencyParams(9, 6), // 85.3333333333333
    NoiseFrequencyParams(9, 5), // 102.4
    NoiseFrequencyParams(9, 4), // 128
    NoiseFrequencyParams(8, 7), // 146.285714285714
    NoiseFrequencyParams(8, 6), // 170.666666666667
    NoiseFrequencyParams(8, 5), // 204.8
    NoiseFrequencyParams(8, 4), // 256
    NoiseFrequencyParams(7, 7), // 292.571428571429
    NoiseFrequencyParams(7, 6), // 341.333333333333
    NoiseFrequencyParams(7, 5), // 409.6
    NoiseFrequencyParams(7, 4), // 512
    NoiseFrequencyParams(6, 7), // 585.142857142857
    NoiseFrequencyParams(6, 6), // 682.666666666667
    NoiseFrequencyParams(6, 5), // 819.2
    NoiseFrequencyParams(6, 4), // 1024
    NoiseFrequencyParams(5, 7), // 1170.28571428571
    NoiseFrequencyParams(5, 6), // 1365.33333333333
    NoiseFrequencyParams(5, 5), // 1638.4
    NoiseFrequencyParams(5, 4), // 2048
    NoiseFrequencyParams(4, 7), // 2340.57142857143
    NoiseFrequencyParams(4, 6), // 2730.66666666667
    NoiseFrequencyParams(4, 5), // 3276.8
    NoiseFrequencyParams(4, 4), // 4096
    NoiseFrequencyParams(3, 7), // 4681.14285714286
    NoiseFrequencyParams(3, 6), // 5461.33333333333
    NoiseFrequencyParams(3, 5), // 6553.6
    NoiseFrequencyParams(3, 4), // 8192
    NoiseFrequencyParams(2, 7), // 9362.28571428571
    NoiseFrequencyParams(2, 6), // 10922.6666666667
    NoiseFrequencyParams(2, 5), // 13107.2
    NoiseFrequencyParams(2, 4), // 16384
    NoiseFrequencyParams(1, 7), // 18724.5714285714
    NoiseFrequencyParams(1, 6), // 21845.3333333333
    NoiseFrequencyParams(1, 5), // 26214.4
    NoiseFrequencyParams(1, 4), // 32768
    NoiseFrequencyParams(0, 7), // 37449.1428571429
    NoiseFrequencyParams(0, 6), // 43690.6666666667
    NoiseFrequencyParams(0, 5), // 52428.8
    NoiseFrequencyParams(0, 4), // 65536
    NoiseFrequencyParams(0, 3), // 87381.3333333333
    NoiseFrequencyParams(0, 2), // 131072
    NoiseFrequencyParams(0, 1), // 262144
    NoiseFrequencyParams(0, 0), // 524288
};

static const size_t NOISE_PARAM_TABLE_LEN = sizeof(NOISE_PARAM_TABLE) / sizeof(NoiseFrequencyParams);
