
#include "HEVCAnalyzerFFmpeg.hpp"

const char* HEVCAnalyzerFFmpeg::getColorRange(AVColorRange color_range) {
    switch (color_range) {
    case AVCOL_RANGE_MPEG: return "MPEG";
    case AVCOL_RANGE_JPEG: return "JPEG";
    case AVCOL_RANGE_UNSPECIFIED:
    default: return "unknown";
    }
}

StreamInfo HEVCAnalyzerFFmpeg::analyze(const char* filename) {
    AVFormatContext* fmntCtx = nullptr;
    StreamInfo info = { nullptr, 0, 0, 0.0, 0, 0, 0, 0, nullptr };

    if (avformat_open_input(&fmntCtx, filename, nullptr, nullptr) < 0) {
        std::cerr << "Could not open source file " << filename << std::endl;
        return info;
    }

    if (avformat_find_stream_info(fmntCtx, nullptr) < 0) {
        std::cerr << "Could not find stream information" << std::endl;
        avformat_close_input(&fmntCtx);
        return info;
    }

    const AVCodec* codec = nullptr;
    int streamIndex = av_find_best_stream(fmntCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (streamIndex < 0) {
        std::cerr << "Could not find a video stream in the input file" << std::endl;
        avformat_close_input(&fmntCtx);
        return info;
    }

    AVStream* stream = fmntCtx->streams[streamIndex];
    AVCodecParameters* codecpar = stream->codecpar;

    info.codecName = avcodec_get_name(codecpar->codec_id);
    info.width = codecpar->width;
    info.height = codecpar->height;
    info.frameRate = av_q2d(stream->avg_frame_rate);
    info.duration = fmntCtx->duration;
    info.bitRate = fmntCtx->bit_rate;
    info.profile = codecpar->profile;
    info.level = codecpar->level;
    info.colorRange = getColorRange(codecpar->color_range);

    avformat_close_input(&fmntCtx);
    std::cout << "Printing video stream information collected with the use of FFmpeg library! " << std::endl;
    std::cout << "Code name: " << info.codecName << std::endl;
    std::cout << "Width: " << info.width << ", Height: " << info.height << std::endl;
    std::cout << "Frame rate: " << info.frameRate << std::endl;
    std::cout << "Duration: " << info.duration << std::endl;
    std::cout << "Bit rate: " << info.bitRate << std::endl;
    std::cout << "Profile: " << info.profile << std::endl;
    std::cout << "Level: " << info.level << std::endl;
    std::cout << "Color Range: " << info.colorRange << std::endl;

    return info;
}
