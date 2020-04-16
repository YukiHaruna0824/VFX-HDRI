#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<cmath>

#include<random>
#include<functional>

#include<opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class ImageUtils
{
public:
	void parseImageInfo(std::string rootFolder);
	void gSamplePixel(int nsample);

	int getWeight(int value);
	void gSolve(float lambda_rate);
	void getRadianceMap();
	void toneMappingByReinhard(float lumin);

	void writeGCurveData(std::string filepath);
	void writeHdr(std::string filepath);
	void writeLdrImage(std::string filepath);

	void setAlignImages(std::vector<cv::Mat> alignImages);
	std::vector<cv::Mat>& getImages();

private:
	std::vector<cv::Mat> images;
	std::vector<float> exposureTimes;
	std::vector<float> logexposureTimes;

	cv::Mat radianceMap;

	cv::Mat ldr_image;

	int sampleCount = 0;
	int ***samplePixel = nullptr;
	float gFunc[3][256] = { 0 };
};