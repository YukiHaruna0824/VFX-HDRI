#include "ImageUtils.h"

void ImageUtils::parseImageInfo(std::string rootFolder)
{
	std::string infoPath = rootFolder + "/" + "info.txt";
	std::fstream fs;
	fs.open(infoPath, std::ios::in);
	if (fs)
	{
		std::string imageName;
		float exposureTime = 0.0f;
		while (fs >> imageName >> exposureTime)
		{
			//std::cout << imageName << " " << exposureTime << std::endl;
			std::string imagePath = rootFolder + "/" + imageName;
			cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
			this->images.push_back(image.clone());
			this->exposureTimes.push_back(exposureTime);
			this->logexposureTimes.push_back(log(exposureTime));
		}
	}
	else
		std::cout << "Open File Failed!" << std::endl;

	fs.close();
}

void ImageUtils::gSamplePixel(int nsample)
{
	this->sampleCount = nsample;
	this->samplePixel = new int**[nsample];
	for (int i = 0; i < nsample; i++) {
		this->samplePixel[i] = new int*[this->images.size()];
		for (int j = 0; j < this->images.size(); j++) {
			this->samplePixel[i][j] = new int[3];
			for (int k = 0; k < 3; k++)
				this->samplePixel[i][j][k] = 0;
		}
	}

	//take first image to get w, h
	int height = this->images[0].rows;
	int width = this->images[0].cols;

	//random sample
	/*
	std::random_device rd;
	std::mt19937 gen = std::mt19937(rd());
	std::uniform_real_distribution<> ruh(0, height);
	std::uniform_real_distribution<> ruw(0, width);
	auto randH = std::bind(ruh, gen);
	auto randW = std::bind(ruw, gen);
	for (int i = 0; i < nsample; i++){
		for (int j = 0; j < this->images.size(); j++) {
			cv::Mat image = this->images[j];
			for (int k = 0; k < 3; k++) {
				this->samplePixel[i][j][k] = image.at<cv::Vec3b>(randH(), randW())[k];
			}
		}
	}
	*/

	//uniform sample
	int sq_sample = sqrt(nsample);
	int unitH = height / sq_sample;
	int unitW = width / sq_sample;
	for (int i = 0; i < nsample; i++) {
		int sw = (i % sq_sample) * unitW;
		int sh = (i / sq_sample) * unitH;
		for (int j = 0; j < this->images.size(); j++) {
			cv::Mat image = this->images[j];
			for (int k = 0; k < 3; k++)
				this->samplePixel[i][j][k] = image.at<cv::Vec3b>(sh, sw)[k];
		}
	}

	std::cout << "Uniform Sample Pixel End!" << std::endl;
}

int ImageUtils::getWeight(int value)
{
	return value > 127 ? 255 - value : value;
}

void ImageUtils::gSolve(float lambda_rate)
{
	for (int channel = 0; channel < 3; channel++) {
		int n = 254;
		cv::Mat A = cv::Mat::zeros(this->sampleCount * this->images.size() + n + 1, this->sampleCount + 256, CV_32F);
		cv::Mat b = cv::Mat::zeros(this->sampleCount * this->images.size() + n + 1, 1, CV_32F);

		int k = 0;
		for (int i = 0; i < this->sampleCount; i++) {
			for (int j = 0; j < this->images.size(); j++) {
				int weight = this->getWeight(this->samplePixel[i][j][channel]);
				A.at<float>(k, this->samplePixel[i][j][channel]) = weight;
				A.at<float>(k, 256 + i) = -weight;
				b.at<float>(k, 0) = this->logexposureTimes[j] * weight;
				k++;
			}
		}

		// give constraint to get one g function
		A.at<float>(k, 127) = 1;
		k++;

		for (int i = 0; i < n; i++) {
			int weight = this->getWeight(i + 1);
			A.at<float>(k, i) = lambda_rate * weight;
			A.at<float>(k, i + 1) = -2 * lambda_rate * weight;
			A.at<float>(k, i + 2) = lambda_rate * weight;
			k++;
		}

		cv::Mat x = cv::Mat(this->sampleCount + 256, 1, CV_32F);
		cv::solve(A, b, x, cv::DECOMP_SVD);

		//set g Function
		for (int i = 0; i < 256; i++) {
			gFunc[channel][i] = x.at<float>(i, 0);
		}
	}
	std::cout << "gSolve End!" << std::endl;
}

void ImageUtils::getRadianceMap()
{
	//take first image to get w, h
	int height = this->images[0].rows;
	int width = this->images[0].cols;
	
	this->radianceMap = cv::Mat(height, width, CV_32FC3);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			for (int channel = 0; channel < 3; channel++) {
				double a = 0, b = 0;
				for (int k = 0; k < this->images.size(); k++) {
					int zij = this->images[k].at<cv::Vec3b>(i, j)[channel];
					int weight = this->getWeight(zij);
					a += weight * (this->gFunc[channel][zij] - this->logexposureTimes[k]);
					b += weight;
				}
				this->radianceMap.at<cv::Vec3f>(i, j)[channel] = (float)exp(a / (b + 0.1));
			}
		}
	}
	std::cout << "Radiance Map End!" << std::endl;
}

void ImageUtils::toneMappingByReinhard(float lumin)
{
	int height = this->radianceMap.rows;
	int width = this->radianceMap.cols;

	cv::Mat grayRadianceMap = cv::Mat(height, width, CV_32F);
	
	//get mean Lw
	float Lw = 0;
	float delta = pow(10, -6);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			//bgr mode
			cv::Vec3f color = this->radianceMap.at<cv::Vec3f>();
			float gray = color[2] * 0.299f + color[1] * 0.587f + color[0] * 0.114f;
			grayRadianceMap.at<float>(i, j) = gray;
			Lw += log(gray + delta);
		}
	}
	Lw = exp(Lw / (height * width));

	//get Ld / Lw
	float Lwhite = 1.0f;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			float Lm = grayRadianceMap.at<float>(i, j) * lumin / Lw;
			float Ld = Lm * (1 + Lm / pow(Lwhite, 2)) / (1 + Lm);
			grayRadianceMap.at<float>(i, j) = Ld / (grayRadianceMap.at<float>(i, j) + delta);
		}
	}

	ldr_image = cv::Mat(height, width, CV_32FC3);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			ldr_image.at<cv::Vec3f>(i, j) = this->radianceMap.at<cv::Vec3f>(i, j) * grayRadianceMap.at<float>(i, j) * 255.0f;
			for (int k = 0; k < 3; k++)
			{
				if (ldr_image.at<cv::Vec3f>(i, j)[k] > 255.0f)
					ldr_image.at<cv::Vec3f>(i, j)[k] = 255.0f;
			}
		}
	}

	std::cout << "Tone Mapping End!" << std::endl;
}

void ImageUtils::writeGCurveData(std::string filepath)
{
	std::fstream fs;
	fs.open(filepath, std::ios::out);
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 3; j++) {
			fs << this->gFunc[j][i];
			if (j != 2)
				fs << ",";
		}
		fs << std::endl;
	}
	fs.close();
}

void ImageUtils::writeHdr(std::string filepath)
{
	// write .hdr file
	cv::imwrite(filepath, this->radianceMap);
}

void ImageUtils::writeLdrImage(std::string filepath)
{
	cv::imwrite(filepath, this->ldr_image);
}

void ImageUtils::setAlignImages(std::vector<cv::Mat> alignImages)
{
	this->images.clear();
	this->images = alignImages;
}

std::vector<cv::Mat>& ImageUtils::getImages()
{
	return this->images;
}