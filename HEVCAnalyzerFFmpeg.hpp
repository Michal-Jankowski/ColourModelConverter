
#ifndef HEVC_ANALYZERFFMPEG_H
#define HEVC_ANALYZERFFMPEG_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

#include <iostream>

struct StreamInfo {
	const char* codecName;
	int width;
	int height;
	double frameRate;
	int64_t duration;
	int64_t bitRate;
	int profile;
	int level;
	const char* colorRange;
};


class HEVCAnalyzerFFmpeg {
public:
	StreamInfo analyze(const char* filename);

private:
	const char* getColorRange(AVColorRange colorRange);
};

#endif
