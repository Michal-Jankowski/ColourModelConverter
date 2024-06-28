#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>


#include "HEVCParser.hpp"
#include "HEVCAnalyzerFFmpeg.hpp"
#include "ColorConversion.hpp"
#include "VideoConverter.hpp"


void print_usage() {
	std::cout << "Usage: program -i <movie_file> -image <image_file> -o <output_file> [-convert] [-analzye--binary] [-analyze--ffmpeg]" << std::endl;
}


int main(int argc, char* argv[]) {

	
	std::string filenameMovie;
	std::string filenameImg;
	std::string fileOutput;
	bool useVideoConverter = false;
	bool useHEVCParser = false;
	bool useHEVCAnalyzerFFmpeg = false;
	bool useRGB_YUVConversion = false;


	std::vector<std::string> args(argv, argv + argc);



	for (size_t i = 1; i < args.size(); ++i) {
		if (args[i] == "-i" && i + 1 < args.size()) {
			filenameMovie = args[++i];
		}
		else if (args[i] == "-o" && i + 1 < args.size()) {
			fileOutput = args[++i];
		}
		else if (args[i] == "-image" && i + 1 < args.size()) {
			filenameImg = args[++i];
			useRGB_YUVConversion = true;
		}
		else if (args[i] == "-convert") {
			useVideoConverter = true;
		}
		else if (args[i] == "-analzye--binary") {
			useHEVCParser = true;
		}
		else if (args[i] == "-analyze--ffmpeg") {
			useHEVCAnalyzerFFmpeg = true;
		}
		else {
			print_usage();
			return 1;
		}
	}

	if (filenameMovie.empty()) {
		std::cerr << "Movie file must be specified with -i" << std::endl;
		return 1;
	}

	if (fileOutput.empty()) {
		std::cerr << "Output file must be specified with -o" << std::endl;
		return 1;
	}

	if (useVideoConverter) {

		/* non HEVC data stream to HEVC data stream */

		// convert non hevc video stream to hevc video stream
		VideoConverter converter(filenameMovie, fileOutput);
		converter.convertToHEVC();
	}

	if (useHEVCParser) {
		// extract information from HEVC reading NAL structure with binary reading and also with the help of ffmpeg
		try {
			HEVCParser parser(fileOutput);
			HEVCInfo info = parser.parse();
		}
		catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << std::endl;
			return 1;
		}
	}

	if (useHEVCAnalyzerFFmpeg) {
		HEVCAnalyzerFFmpeg ffmpegAnalyzer;
		ffmpegAnalyzer.analyze(fileOutput.c_str());
	}

	if (useRGB_YUVConversion) {

		/* RGB to YUV color space conversion */

		if (filenameImg.empty()) {
			std::cerr << "Could not open or find the image!\n";
			return -1;
		}

		cv::Mat rgbImage = cv::imread(filenameImg);

		if (rgbImage.empty()) {
			std::cerr << "Could not open or find the image!\n";
			return -1;
		}

		cv::Mat yuvImage;
		ColorConversion::convertRGBtoYUV_JPEG(rgbImage, yuvImage);

		// Save the converted YUV image to disk
		if (!cv::imwrite("RGB_to_YUV_img_converted.jpg", yuvImage)) {
			std::cerr << "Failed to save the image!\n";
			return -1;
		}

		std::cout << "Image successfully saved!\n";

		// Display the images before and after conversion
		cv::Mat concatenatedImage;
		cv::hconcat(rgbImage, yuvImage, concatenatedImage);
		cv::imshow("RGB to YUV Conversion", concatenatedImage);
		cv::waitKey(0); // Wait for a keystroke in the window
	}
	return 0;

}