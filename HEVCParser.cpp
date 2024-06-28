
#include "HEVCParser.hpp"
#include <iostream>
#include <stdexcept>

HEVCParser::HEVCParser(const std::string& filename) : filename(filename) {}

HEVCInfo HEVCParser::parse() {
    AVFormatContext* formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) != 0) {
        throw std::runtime_error("Failed to open file");
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        throw std::runtime_error("Failed to retrieve stream info");
    }

    AVCodecParameters* codecParams = nullptr;
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            codecParams = formatContext->streams[i]->codecpar;
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        avformat_close_input(&formatContext);
        throw std::runtime_error("No video stream found");
    }

    if (codecParams->codec_id != AV_CODEC_ID_HEVC) {
        avformat_close_input(&formatContext);
        throw std::runtime_error("Video stream is not HEVC");
    }

    HEVCInfo info = parseHevcSps(codecParams->extradata, codecParams->extradata_size);

    avformat_close_input(&formatContext);

    std::cout << "Width: " << info.width << ", Height: " << info.height << std::endl;
    std::cout << "Profile Space: " << info.profileSpace << ", Tier Flag: " << info.tierFlag << std::endl;
    std::cout << "Profile IDC: " << info.profileIdc << ", Level IDC: " << info.levelIdc << std::endl;
    std::cout << "Chroma Format IDC: " << info.chromaFormatIdc << ", Bit Depth Luma: " << info.bitDepthLuma << std::endl;
    std::cout << "Bit Depth Chroma: " << info.bitDepthChroma << std::endl;

    return info;
}

uint32_t HEVCParser::extractBits(const std::vector<uint8_t>& buffer, size_t& bitOffset, size_t numBits) {
    uint32_t value = 0;
    for (size_t i = 0; i < numBits; ++i) {
        value <<= 1;
        size_t byteOffset = bitOffset / 8;
        size_t bitInByte = 7 - (bitOffset % 8);
        if (buffer[byteOffset] & (1 << bitInByte)) {
            value |= 1;
        }
        ++bitOffset;
    }
    return value;
}

uint32_t HEVCParser::readExpGolombCode(const std::vector<uint8_t>& buffer, size_t& bitOffset) {
    size_t leadingZeroBits = 0;
    while (extractBits(buffer, bitOffset, 1) == 0) {
        ++leadingZeroBits;
    }
    uint32_t codeNum = (1 << leadingZeroBits) - 1 + extractBits(buffer, bitOffset, leadingZeroBits);
    return codeNum;
}

//Reference: https://www.itu.int/rec/T-REC-H.265
HEVCInfo HEVCParser::parseHevcSps(const uint8_t* data, size_t size) {
    std::vector<uint8_t> buffer(data, data + size);
    size_t bitOffset = 0;

    // Skip the NAL unit header
    bitOffset += 4 * 8;

    HEVCInfo info;
    int spsVideoParameterSetId = extractBits(buffer, bitOffset, 4);
    int spsMaxSubLayersMinus1 = extractBits(buffer, bitOffset, 3);
    int spsTemporalIdNestingFlag = extractBits(buffer, bitOffset, 1);

    // Parse profile tier level
    info.profileSpace = extractBits(buffer, bitOffset, 2);
    info.tierFlag = extractBits(buffer, bitOffset, 1);
    info.profileIdc = extractBits(buffer, bitOffset, 5);
    bitOffset += 32; // Skipping some profile compatibility flags
    bitOffset += 16; // Skipping more flags
    info.levelIdc = extractBits(buffer, bitOffset, 8);

    int spsSeqParameterSetId = readExpGolombCode(buffer, bitOffset);

    info.chromaFormatIdc = readExpGolombCode(buffer, bitOffset);
    if (info.chromaFormatIdc == 3) {
        extractBits(buffer, bitOffset, 1); // separate_colour_plane_flag
    }

    int picWidthInLumaSamples = readExpGolombCode(buffer, bitOffset);
    int picHeightInLumaSamples = readExpGolombCode(buffer, bitOffset);

    bool conformanceWindowFlag = extractBits(buffer, bitOffset, 1);
    if (conformanceWindowFlag) {
        int confWinLeftOffset = readExpGolombCode(buffer, bitOffset);
        int confWinRightOffset = readExpGolombCode(buffer, bitOffset);
        int confWinTopOffset = readExpGolombCode(buffer, bitOffset);
        int confWinBottomOffset = readExpGolombCode(buffer, bitOffset);

        info.width = picWidthInLumaSamples - (confWinLeftOffset + confWinRightOffset);
        info.height = picHeightInLumaSamples - (confWinTopOffset + confWinBottomOffset);
    }
    else {
        info.width = picWidthInLumaSamples;
        info.height = picHeightInLumaSamples;
    }

    info.bitDepthLuma = readExpGolombCode(buffer, bitOffset) + 8;
    info.bitDepthChroma = readExpGolombCode(buffer, bitOffset) + 8;

    return info;
}