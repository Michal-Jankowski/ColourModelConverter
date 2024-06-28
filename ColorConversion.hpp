#ifndef COLORCONVERSION_H
#define COLORCONVERSION_H

#include <opencv2/opencv.hpp>
#include <iostream>

struct RGB {
    double r, g, b;
};

struct YUV {
    double y, u, v;
};

template <typename T>
inline YUV rgbToYuv(RGB rgb) {
    YUV yuv;

    yuv.y = T::yr * rgb.r + T::yg * rgb.g + T::yb * rgb.b;
    yuv.u = T::ur * rgb.r + T::ug * rgb.g + T::ub * rgb.b;
    yuv.v = T::vr * rgb.r + T::vg * rgb.g + T::vb * rgb.b;

    return yuv;
}

template <typename T>
inline RGB yuvToRgb(YUV yuv) {
    RGB rgb;

    rgb.r = yuv.y + 1.402 * (yuv.v - 128);
    rgb.g = yuv.y - 0.344136 * (yuv.u - 128) - 0.714136 * (yuv.v - 128);
    rgb.b = yuv.y + 1.772 * (yuv.u - 128);

    return rgb;
}
//Reference: https://en.wikipedia.org/wiki/Y%E2%80%B2UV
// BT.2020 (Rec. 2020)
// Usage:
// Ultra High Definition TV = UHDTV
struct BT2020 {
    static constexpr double yr = 0.2627;
    static constexpr double yg = 0.6780;
    static constexpr double yb = 0.0593;
    static constexpr double ur = -0.13963;
    static constexpr double ug = -0.36037;
    static constexpr double ub = 0.5;
    static constexpr double vr = 0.5;
    static constexpr double vg = -0.45979;
    static constexpr double vb = -0.04021;
};

struct JPEG {
    static constexpr double yr = 0.299;
    static constexpr double yg = 0.587;
    static constexpr double yb = 0.114;
    static constexpr double ur = -0.168736; // Cbr
    static constexpr double ug = -0.331264; // Cbg
    static constexpr double ub = 0.5; // Cbb
    static constexpr double vr = 0.5;
    static constexpr double vg = -0.418688;
    static constexpr double vb = -0.081312;
};

class ColorConversion {
public:
    static void convertRGBtoYUV_BT2020(const cv::Mat& rgbImage, cv::Mat& yuvImage);
    static void convertRGBtoYUV_JPEG(const cv::Mat& rgbImage, cv::Mat& yuvImage);
    static void convertRGBtoYUV(const cv::Mat& rgbImage, cv::Mat& yuvImage);
};

#endif // COLORCONVERSION_H