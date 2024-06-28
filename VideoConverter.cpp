#include "VideoConverter.hpp"

// Opens the input file and prepares the format context.
bool VideoConverter::openInputFile() {
    // Allocate an AVFormatContext for the input file.
    inputFormatContext = avformat_alloc_context();
    if (avformat_open_input(&inputFormatContext, inputFilename.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Could not open input file: " << inputFilename << std::endl;
        return false;
    }

    if (avformat_find_stream_info(inputFormatContext, nullptr) < 0) {
        std::cerr << "Could not find stream information in file: " << inputFilename << std::endl;
        return false;
    }

    av_dump_format(inputFormatContext, 0, inputFilename.c_str(), 0);
    return true;
}

// Initializes the decoder contexts for both video and audio streams.
bool VideoConverter::initializeDecoderContexts() {
    const AVCodec* videoDecoder = nullptr;
    int videoStreamIndex = av_find_best_stream(inputFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &videoDecoder, 0);
    if (videoStreamIndex < 0) {
        std::cerr << "Could not find video stream in the input file." << std::endl;
        return false;
    }

    const AVCodec* audioDecoder = nullptr;
    int audioStreamIndex = av_find_best_stream(inputFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &audioDecoder, 0);
    if (audioStreamIndex < 0) {
        std::cerr << "Could not find audio stream in the input file." << std::endl;
        return false;
    }

    videoStream = inputFormatContext->streams[videoStreamIndex];
    audioStream = inputFormatContext->streams[audioStreamIndex];

    videoDecoderContext = avcodec_alloc_context3(videoDecoder);
    avcodec_parameters_to_context(videoDecoderContext, videoStream->codecpar);
    if (avcodec_open2(videoDecoderContext, videoDecoder, nullptr) < 0) {
        std::cerr << "Could not open video codec." << std::endl;
        return false;
    }

    audioDecoderContext = avcodec_alloc_context3(audioDecoder);
    avcodec_parameters_to_context(audioDecoderContext, audioStream->codecpar);
    if (avcodec_open2(audioDecoderContext, audioDecoder, nullptr) < 0) {
        std::cerr << "Could not open audio codec." << std::endl;
        return false;
    }

    return true;
}

// Writes the header for the output file.
bool VideoConverter::writeOutputContext() {

    if (!(outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&outputFormatContext->pb, outputFilename.c_str(), AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Could not open output file." << std::endl;
            return false;
        }
    }

    if (avformat_write_header(outputFormatContext, nullptr) < 0) {
        std::cerr << "Error occurred when opening output file." << std::endl;
        return false;
    }

    return true;
}

// Initializes the encoder contexts for both video and audio streams.
bool VideoConverter::initializeOutputFile() {
    avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, outputFilename.c_str());
    if (!outputFormatContext) {
        std::cerr << "Could not create output context." << std::endl;
        return false;
    }

    outputVideoStream = avformat_new_stream(outputFormatContext, nullptr);
    if (!outputVideoStream) {
        std::cerr << "Failed to allocate video output stream." << std::endl;
        return false;
    }

    outputAudioStream = avformat_new_stream(outputFormatContext, nullptr);
    if (!outputAudioStream) {
        std::cerr << "Failed to allocate audio output stream." << std::endl;
        return false;
    }

    return true;
}

// Encodes and writes frames from the input file to the output file.
bool VideoConverter::initializeEncoderContexts() {
    const AVCodec* videoEncoder = avcodec_find_encoder(AV_CODEC_ID_HEVC);
    if (!videoEncoder) {
        std::cerr << "Necessary video encoder not found." << std::endl;
        return false;
    }

    videoEncoderContext = avcodec_alloc_context3(videoEncoder);
    videoEncoderContext->height = videoDecoderContext->height;
    videoEncoderContext->width = videoDecoderContext->width;
    videoEncoderContext->sample_aspect_ratio = videoDecoderContext->sample_aspect_ratio;
    videoEncoderContext->pix_fmt = AV_PIX_FMT_YUV420P;
    videoEncoderContext->time_base = av_inv_q(av_guess_frame_rate(inputFormatContext, videoStream, nullptr));
    videoEncoderContext->framerate = av_guess_frame_rate(inputFormatContext, videoStream, nullptr);
    videoEncoderContext->max_b_frames = 5;

    outputVideoStream->time_base = videoEncoderContext->time_base;

    if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        videoEncoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(videoEncoderContext, videoEncoder, nullptr) < 0) {
        std::cerr << "Could not open video encoder." << std::endl;
        return false;
    }

    avcodec_parameters_from_context(outputVideoStream->codecpar, videoEncoderContext);

    const AVCodec* audioEncoder = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audioEncoder) {
        std::cerr << "Necessary audio encoder not found." << std::endl;
        return false;
    }

    audioEncoderContext = avcodec_alloc_context3(audioEncoder);
    audioEncoderContext->sample_rate = audioDecoderContext->sample_rate;
    audioEncoderContext->ch_layout = audioDecoderContext->ch_layout;
    audioEncoderContext->sample_fmt = audioEncoder->sample_fmts[0];
    audioEncoderContext->time_base = { 1, audioEncoderContext->sample_rate };

    outputAudioStream->time_base = audioEncoderContext->time_base;

    if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        audioEncoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(audioEncoderContext, audioEncoder, nullptr) < 0) {
        std::cerr << "Could not open audio encoder." << std::endl;
        return false;
    }

    avcodec_parameters_from_context(outputAudioStream->codecpar, audioEncoderContext);

    return true;
}

// Encodes and writes frames from the input file to the output file.
void VideoConverter::encodeAndWriteFrames() {
    AVPacket* packet = av_packet_alloc();
    while (av_read_frame(inputFormatContext, packet) >= 0) {
        if (packet->stream_index == videoStream->index) {
            avcodec_send_packet(videoDecoderContext, packet);
            AVFrame* frame = av_frame_alloc();
            if (avcodec_receive_frame(videoDecoderContext, frame) == 0) {
                avcodec_send_frame(videoEncoderContext, frame);
                AVPacket* outputPacket = av_packet_alloc();
                if (avcodec_receive_packet(videoEncoderContext, outputPacket) == 0) {
                    outputPacket->stream_index = outputVideoStream->index;
                    av_interleaved_write_frame(outputFormatContext, outputPacket);
                    av_packet_unref(outputPacket);
                }
                av_packet_free(&outputPacket);
            }
            av_frame_free(&frame);
        }
        else if (packet->stream_index == audioStream->index) {
            // Process audio frames.
            avcodec_send_packet(audioDecoderContext, packet);
            AVFrame* frame = av_frame_alloc();
            if (avcodec_receive_frame(audioDecoderContext, frame) == 0) {
                avcodec_send_frame(audioEncoderContext, frame);
                AVPacket* outputPacket = av_packet_alloc();
                if (avcodec_receive_packet(audioEncoderContext, outputPacket) == 0) {
                    outputPacket->stream_index = outputAudioStream->index;
                    av_interleaved_write_frame(outputFormatContext, outputPacket);
                    av_packet_unref(outputPacket);
                }
                av_packet_free(&outputPacket);
            }
            av_frame_free(&frame);
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
}

// Flushes the encoders to ensure all remaining frames are processed.
void VideoConverter::flushEncoders() {
    AVPacket* packet = av_packet_alloc();

    // Flush video encoder
    avcodec_send_frame(videoEncoderContext, nullptr);
    while (avcodec_receive_packet(videoEncoderContext, packet) == 0) {
        packet->stream_index = outputVideoStream->index;
        av_interleaved_write_frame(outputFormatContext, packet);
        av_packet_unref(packet);
    }

    // Flush audio encoder
    avcodec_send_frame(audioEncoderContext, nullptr);
    while (avcodec_receive_packet(audioEncoderContext, packet) == 0) {
        packet->stream_index = outputAudioStream->index;
        av_interleaved_write_frame(outputFormatContext, packet);
        av_packet_unref(packet);
    }

    av_packet_free(&packet);
}

// Cleans up and releases all allocated resources.
void VideoConverter::cleanup() {
    av_write_trailer(outputFormatContext);
    avio_closep(&outputFormatContext->pb);
    avcodec_free_context(&audioEncoderContext);
    avcodec_free_context(&videoEncoderContext);
    avformat_free_context(outputFormatContext);
    avcodec_free_context(&audioDecoderContext);
    avcodec_free_context(&videoDecoderContext);
    avformat_close_input(&inputFormatContext);
}

// Main function to convert the input video to HEVC format.
void VideoConverter::convertToHEVC() {
    if (!openInputFile()) {
        return;
    }
    if (!initializeOutputFile()) {
        cleanup();
        return;
    }
    if (!initializeDecoderContexts()) {
        cleanup();
        return;
    }
    if (!initializeEncoderContexts()) {
        cleanup();
        return;
    }
    if (!writeOutputContext()) {
        cleanup();
        return;
    }

    encodeAndWriteFrames();
    flushEncoders();
    cleanup();
}