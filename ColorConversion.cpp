#include "ColorConversion.hpp"

void ColorConversion::convertRGBtoYUV_BT2020(const cv::Mat& rgbImage, cv::Mat& yuvImage) {
    yuvImage = cv::Mat::zeros(rgbImage.size(), rgbImage.type());
    int rows = rgbImage.rows;
    int cols = rgbImage.cols;
    int channels = rgbImage.channels();
    std::cout << "Image Dimensions: " << rows << " x " << cols << "\n";

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (channels == 3) {
                auto pixelValue = rgbImage.at<cv::Vec3b>(i, j);
                RGB rgb = {
                    pixelValue[0] / 255.0,
                    pixelValue[1] / 255.0,
                    pixelValue[2] / 255.0
                };
                auto yuv = rgbToYuv<BT2020>(rgb);
                yuv.y = yuv.y * 255.0;
                yuv.u = yuv.u * 255.0 + 128.0;
                yuv.v = yuv.v * 255.0 + 128.0;

                yuvImage.at<cv::Vec3b>(i, j)[0] = static_cast<uchar>(yuv.y);
                yuvImage.at<cv::Vec3b>(i, j)[1] = static_cast<uchar>(yuv.u);
                yuvImage.at<cv::Vec3b>(i, j)[2] = static_cast<uchar>(yuv.v);
            }
        }
    }
}

void ColorConversion::convertRGBtoYUV(const cv::Mat& rgbImage, cv::Mat& yuvImage) {
    yuvImage = cv::Mat::zeros(rgbImage.size(), rgbImage.type());
    int rows = rgbImage.rows;
    int cols = rgbImage.cols;
    int channels = rgbImage.channels();
    std::cout << "Image Dimensions: " << rows << " x " << cols << "\n";

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (channels == 3) {
                auto pixelValue = rgbImage.at<cv::Vec3b>(i, j);
                RGB rgb = {
                    static_cast<int>(pixelValue[0]),
                    static_cast<int>(pixelValue[1]),
                    static_cast<int>(pixelValue[2])
                };
                auto yuv = rgbToYuv<BT2020>(rgb);
                yuvImage.at<cv::Vec3b>(i, j)[0] = static_cast<uchar>(yuv.y);
                yuvImage.at<cv::Vec3b>(i, j)[1] = static_cast<uchar>(yuv.u + 128);
                yuvImage.at<cv::Vec3b>(i, j)[2] = static_cast<uchar>(yuv.v + 128);
            }
        }
    }
}

void ColorConversion::convertRGBtoYUV_JPEG(const cv::Mat& rgbImage, cv::Mat& yuvImage) {
    yuvImage = cv::Mat::zeros(rgbImage.size(), rgbImage.type());
    int rows = rgbImage.rows;
    int cols = rgbImage.cols;
    int channels = rgbImage.channels();
    std::cout << "Image Dimensions: " << rows << " x " << cols << "\n";

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (channels == 3) {
                auto pixelValue = rgbImage.at<cv::Vec3b>(i, j);
                RGB rgb = {
                    pixelValue[0] / 255.0,
                    pixelValue[1] / 255.0,
                    pixelValue[2] / 255.0
                };
                auto yuv = rgbToYuv<JPEG>(rgb);
                yuv.y = yuv.y * 255.0;
                yuv.u = yuv.u * 255.0 + 128.0;
                yuv.v = yuv.v * 255.0 + 128.0;

                yuvImage.at<cv::Vec3b>(i, j)[0] = static_cast<uchar>(yuv.y);
                yuvImage.at<cv::Vec3b>(i, j)[1] = static_cast<uchar>(yuv.u);
                yuvImage.at<cv::Vec3b>(i, j)[2] = static_cast<uchar>(yuv.v);
            }
        }
    }
}