#ifndef HEVCPARSER_H
#define HEVCPARSER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include <string>
#include <vector>

struct HEVCInfo {
    int width;
    int height;
    int profileSpace;
    bool tierFlag;
    int profileIdc;
    int levelIdc;
    int chromaFormatIdc;
    int bitDepthLuma;
    int bitDepthChroma;
};

class HEVCParser {
public:
    HEVCParser(const std::string& filename);

    HEVCInfo parse();

private:
    std::string filename;

    uint32_t extractBits(const std::vector<uint8_t>& buffer, size_t& bitOffset, size_t numBits);
    uint32_t readExpGolombCode(const std::vector<uint8_t>& buffer, size_t& bitOffset);
    HEVCInfo parseHevcSps(const uint8_t* data, size_t size);
};

#endif // HEVCPARSER_H