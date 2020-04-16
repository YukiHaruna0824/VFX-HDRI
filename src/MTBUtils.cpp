#include "MTBUtils.h"

void MTBUtils::convertToMTB(std::vector<cv::Mat> images)
{
	for (int i = 0; i < images.size(); i++) {
		int color_count[256] = { 0 };
		int height = images[0].rows;
		int width = images[0].cols;

		cv::Mat mtb_image, exclusive_map;
		cv::cvtColor(images[i], mtb_image, cv::COLOR_BGR2GRAY);
		cv::cvtColor(images[i], exclusive_map, cv::COLOR_BGR2GRAY);

		for (int j = 0; j < height; j++) {
			for (int k = 0; k < width; k++) {
				color_count[mtb_image.at<uchar>(j, k)]++;
			}
		}
	
		int median = width * height / 2;
		int threshold = 0;
		int bias = 20;
		for (int j = 0; j < 256; j++) {
			median -= color_count[j];
			if (median <= 0) {
				threshold = j;
				break;
			}
		}

		//std::cout << threshold << std::endl;

		for (int j = 0; j < height; j++) {
			for (int k = 0; k < width; k++) {
				if (mtb_image.at<uchar>(j, k) > threshold - bias && mtb_image.at<uchar>(j, k) < threshold + bias)
					exclusive_map.at<uchar>(j, k) = 0;
				else
					exclusive_map.at<uchar>(j, k) = 255;
				mtb_image.at<uchar>(j, k) = mtb_image.at<uchar>(j, k) > threshold ? 255 : 0;
			}
		}
		this->mtbImages.push_back(mtb_image.clone());
		this->exclusiveImages.push_back(exclusive_map.clone());
	}
	this->images = images;
}

int MTBUtils::computeDistance(cv::Mat sample_mtb, cv::Mat sample_ex, cv::Mat comp_mtb, cv::Mat comp_ex)
{
	int distance = 0;
	int height = sample_mtb.rows;
	int width = sample_mtb.cols;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (sample_ex.at<uchar>(i, j) != 0 && comp_ex.at<uchar>(i, j) != 0
				&& sample_mtb.at<uchar>(i, j) != comp_mtb.at<uchar>(i, j))
				distance++;
		}
	}
	return distance;
}

cv::Mat MTBUtils::translateImage(cv::Mat src, int dir_x, int dir_y)
{
	if (dir_x == 0 && dir_y == 0)
		return src;

	int height = src.rows;
	int width = src.cols;
	cv::Mat result = src.clone();
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int newH = i - dir_y;
			int newW = j - dir_x;
			newH = std::max(0, std::min(newH, height - 1));
			newW = std::max(0, std::min(newW, width - 1));
			if (src.channels() == 1)
				result.at<uchar>(i, j) = src.at<uchar>(newH, newW);
			else if (src.channels() == 3)
				result.at<cv::Vec3b>(i, j) = src.at<cv::Vec3b>(newH, newW);
		}
	}
	return result;
}

void MTBUtils::imageAlignment(int sampleIndex, int depth)
{
	cv::Mat sample_mtb = this->mtbImages[sampleIndex];
	cv::Mat sample_ex = this->exclusiveImages[sampleIndex];

	for (int i = 0; i < this->mtbImages.size(); i++) {
		std::pair<int, int> allOffset = std::make_pair(0, 0);
		
		if (i != sampleIndex) {
			cv::Mat comp_mtb = this->mtbImages[i];
			cv::Mat comp_ex = this->exclusiveImages[i];

			for (int d = depth; d >= 0; d--) {
				cv::Mat sample_mtb_resize, sample_ex_resize;
				cv::Mat comp_mtb_resize, comp_ex_resize;

				// resize mtb image and exclusive map
				cv::resize(sample_mtb, sample_mtb_resize, cv::Size(pow(2, d), pow(2, d)));
				cv::resize(sample_ex, sample_ex_resize, cv::Size(pow(2, d), pow(2, d)));
				cv::resize(comp_mtb, comp_mtb_resize, cv::Size(pow(2, d), pow(2, d)));
				cv::resize(comp_ex, comp_ex_resize, cv::Size(pow(2, d), pow(2, d)));
				
				//find shift
				int directions[9][2] = { {0, 0}, {0, 1}, {0, -1}, {1, 0}, {1, 1}, {1, -1}, {-1, 0}, {-1, 1}, {-1, -1} };
				int bestDirIndex = 0;
				int minDistance = this->computeDistance(sample_mtb_resize, sample_ex_resize, comp_mtb_resize, comp_ex_resize);
				for (int dir = 1; dir < 9; dir++) {
					sample_mtb_resize = this->translateImage(sample_mtb_resize, directions[dir][0], directions[dir][1]);
					sample_ex_resize = this->translateImage(sample_ex_resize, directions[dir][0], directions[dir][1]);
					comp_mtb_resize = this->translateImage(comp_mtb_resize, directions[dir][0], directions[dir][1]);
					comp_ex_resize = this->translateImage(comp_ex_resize, directions[dir][0], directions[dir][1]);
					
					int distance = this->computeDistance(sample_mtb_resize, sample_ex_resize, comp_mtb_resize, comp_ex_resize);
					if (distance < minDistance) {
						bestDirIndex = dir;
						minDistance = distance;
					}
				}

				allOffset.first = allOffset.first + directions[bestDirIndex][0] * pow(2, d);
				allOffset.second = allOffset.second + directions[bestDirIndex][1] * pow(2, d);
			}
		}

		printf("Image %d shift : ( %d , %d )\n", i, allOffset.first, allOffset.second);
		this->alignImages.push_back(this->translateImage(this->images[i], allOffset.first, allOffset.second));
	}
}

void MTBUtils::writeMtbImages(std::string rootFolder)
{
	for (int i = 0; i < this->mtbImages.size(); i++) {
		std::string filepath = rootFolder + "/mtb" + std::to_string(i) + ".jpg";
		cv::imwrite(filepath, this->mtbImages[i]);
	}
}

void MTBUtils::writeExImages(std::string rootFolder)
{
	for (int i = 0; i < this->exclusiveImages.size(); i++) {
		std::string filepath = rootFolder + "/ex" + std::to_string(i) + ".jpg";
		cv::imwrite(filepath, this->exclusiveImages[i]);
	}
}

void MTBUtils::writeAlignImages(std::string rootFolder)
{
	for (int i = 0; i < this->alignImages.size(); i++) {
		std::string filepath = rootFolder + "/align" + std::to_string(i) + ".jpg";
		cv::imwrite(filepath, this->alignImages[i]);
	}
}

std::vector<cv::Mat>& MTBUtils::getAlignImages()
{
	return this->alignImages;
}