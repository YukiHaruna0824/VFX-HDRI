#include <iostream>
#include "ImageUtils.h"
#include "MTBUtils.h"

#include <io.h>
#include <direct.h>

#include<opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::cout << "argument error !" << std::endl;
		return 0;
	}

	std::string inputFolder = argv[1];

	std::string mtbFolder = "./mtb";
	int n = _access(mtbFolder.c_str(), 0);
	if (n == -1)
		_mkdir(mtbFolder.c_str());

	std::string radianceMapName = "./hdr.hdr";
	std::string gCurveDataName = "./responseCurve.csv";
	std::string ldr_imageName = "./ldr.jpg";

	float lumin = 0.15;
	if (argc > 2) {
		lumin = std::stof(argv[2]);
	}

	int depth = 0;
	if (argc > 3) {
		depth = std::stof(argv[3]);
	}

	ImageUtils imgUtils;
	MTBUtils mtbUtils;
	
	//get image data
	imgUtils.parseImageInfo(inputFolder);
	std::vector<cv::Mat> images = imgUtils.getImages();
	
	//image alignment
	mtbUtils.convertToMTB(images);
	mtbUtils.writeMtbImages(mtbFolder);
	mtbUtils.writeExImages(mtbFolder);
	mtbUtils.imageAlignment(images.size() / 2, depth);
	mtbUtils.writeAlignImages(mtbFolder);

	//convert to hdr
	std::vector<cv::Mat> alignImages = mtbUtils.getAlignImages();
	imgUtils.setAlignImages(alignImages);
	imgUtils.gSamplePixel(225);
	imgUtils.gSolve(10);
	imgUtils.writeGCurveData(gCurveDataName);
	imgUtils.getRadianceMap();
	imgUtils.writeHdr(radianceMapName);

	//tonemapping
	imgUtils.toneMappingByReinhard(lumin);
	imgUtils.writeLdrImage(ldr_imageName);

	return 0;
}