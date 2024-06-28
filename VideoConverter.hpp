
#ifndef VIDECONVERTER_H
#define VIDECONVERTER_H

#include <iostream>
#include <string>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}

class VideoConverter {
public:
    VideoConverter(const std::string& inputFilename, const std::string& outputFilename)
        : inputFilename(inputFilename), outputFilename(outputFilename) 
    {
        av_log_set_level(AV_LOG_DEBUG);
    }
    void convertToHEVC();

private:
    std::string inputFilename;
    std::string outputFilename;

    AVFormatContext* inputFormatContext = nullptr;
    AVCodecContext* videoDecoderContext = nullptr;
    AVCodecContext* audioDecoderContext = nullptr;
    AVCodecContext* videoEncoderContext = nullptr;
    AVCodecContext* audioEncoderContext = nullptr;
    AVFormatContext* outputFormatContext = nullptr;
    AVStream* videoStream = nullptr;
    AVStream* audioStream = nullptr;
    AVStream* outputVideoStream = nullptr;
    AVStream* outputAudioStream = nullptr;

    bool openInputFile();
    bool initializeDecoderContexts();
    bool initializeOutputFile();
    bool initializeEncoderContexts();
    bool writeOutputContext();
    void encodeAndWriteFrames();
    void flushEncoders();
    void cleanup();
public:

    VideoConverter() = default;
};

#endif