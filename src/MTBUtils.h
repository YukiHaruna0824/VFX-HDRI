#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<cmath>

#include<opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class MTBUtils
{
public:
	void convertToMTB(std::vector<cv::Mat> images);
	int computeDistance(cv::Mat sample_mtb, cv::Mat sample_ex, cv::Mat comp_mtb, cv::Mat comp_ex);
	void imageAlignment(int sampleIndex, int depth);

	void writeMtbImages(std::string rootFolder);
	void writeExImages(std::string rootFolder);
	void writeAlignImages(std::string rootFolder);

	std::vector<cv::Mat>& getAlignImages();
	cv::Mat translateImage(cv::Mat src, int dir_x, int dir_y);

private:
	std::vector<cv::Mat> images;
	std::vector<cv::Mat> mtbImages;
	std::vector<cv::Mat> exclusiveImages;

	std::vector<cv::Mat> alignImages;
};

