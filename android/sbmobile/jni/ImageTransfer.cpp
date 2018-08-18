#include "ImageTransfer.h"
#include <algorithm>
#include <sb/SBUtilities.h>
#include <sr/sr_mat.h>

//#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"



//inline float clamp(float x, float a, float b) { return std::max(a, std::min(b, x)); }
inline float saturate(float x) { 
	//return clamp(x, 0.f, 1.f); 
	return std::max(0.f, std::min(1.f, x));
}

void lab2rgb(float* lab, unsigned char* rgb, unsigned int size)
{
	float temp1[16] = { 1, 1, 1, 0,
		1, 1, -1, 0,
		1, -2, 0, 0,
		0, 0,  0, 0 };
	float temp2[16] = { sqrt(3.f) / 3.f, 0, 0, 0,
		0, sqrt(6.f) / 6.f, 0 , 0,
		0, 0, sqrt(2.f) / 2.f , 0,
		0, 0,  0,  0 };
	SrMat m1_lab2rgb = SrMat(temp1);
	SrMat m2_lab2rgb = SrMat(temp2);

	float aLMS2RGB[16] = { 4.4679f, -3.5873f, 0.1193f , 0,
		-1.2186f, +2.3809f, -0.1624f, 0,
		0.0497f, -0.2439f, 1.2045f , 0,
		0,        0,        0,     0 };
	SrMat mLMS2RGB = SrMat(aLMS2RGB);

	for (unsigned int i = 0; i < size; i = i + 4)
	{
		float l = lab[i];
		float a = lab[i + 1];
		float b = lab[i + 2];
		SrVec lab(l, a, b);

		//Convert to LMS
		SrVec LMS = m1_lab2rgb * m2_lab2rgb * lab;
		LMS = SrVec(pow(10, LMS.x), pow(10, LMS.y), pow(10, LMS.z));//pow(SrVec(10), LMS);
																	//Convert to RGB
		auto rgbv = mLMS2RGB * LMS;
		rgb[i] = static_cast<unsigned char>(255.*saturate(rgbv.x));
		rgb[i + 1] = static_cast<unsigned char>(255.*saturate(rgbv.y));
		rgb[i + 2] = static_cast<unsigned char>(255.*saturate(rgbv.z));
	}
}

void rgb2lab(unsigned char* rgb, float* lab, unsigned int size)
{
	float temp1[16] = { 1.f / sqrt(3.f) , 0, 0, 0,
		0, 1.f / sqrt(6.f), 0 , 0,
		0, 0, 1.f / sqrt(2.f), 0,
		0, 0, 0, 0 };
	float temp2[16] = { 1, 1, 1, 0,
		1, 1, -2, 0,
		1, -1, 0, 0,
		0, 0, 0, 0 };

	SrMat m1_rgb2lab = SrMat(temp1);
	SrMat m2_rgb2lab = SrMat(temp2);

	float aRGB2LMS[16] = { 0.3811f, 0.5783f, 0.0402f, 0,
		0.1967f, 0.7244f, 0.0782f, 0,
		0.0241f, 0.1288f, 0.8444f, 0,
		0,       0,       0,       0 };
	SrMat tmRGB2LMS = SrMat(aRGB2LMS);

	for (unsigned int i = 0; i < size; i = i + 4)
	{
		// log10(0) = -inf, so gotta use FLT_TRUE_MIN
		// otherwise, image stats are fucked up
		float r = std::max(FLT_MIN, rgb[i] / 255.f);
		float g = std::max(FLT_MIN, rgb[i + 1] / 255.f);
		float b = std::max(FLT_MIN, rgb[i + 2] / 255.f);
		SrVec rgb(r, g, b);

		//Convert to LMS
		SrVec LMS = tmRGB2LMS * rgb;
		LMS = SrVec(log10(LMS.x), log10(LMS.y), log10(LMS.z));

		//Convert to lab
		SrVec labv = m1_rgb2lab * m2_rgb2lab  * LMS;

		lab[i] = labv.x;
		lab[i + 1] = labv.y;
		lab[i + 2] = labv.z;
	}
}

void computeImageMeanAndStd(float* lab, unsigned char* mask, int imgSize, SrVec& outMean, SrVec& outStd)
{
	int pixCount = 0;
	SrVec imgMean = SrVec(0, 0, 0), imgStd = SrVec(0, 0, 0);
	for (unsigned int i = 0; i < imgSize; i++)
	{
		int labIdx = i * 4;
		if (mask[i] == 0) continue;
		SrVec pix = SrVec(lab[labIdx], lab[labIdx + 1], lab[labIdx + 2]);
		//SmartBody::util::log("Pix %d = %s", i, pix.toString().c_str());
		imgMean += pix;
		pixCount++;
	}

	//SmartBody::util::log("Image Pix Count = %d", pixCount);
	imgMean /= pixCount;

	for (unsigned int i = 0; i < imgSize; i++)
	{
		int labIdx = i * 4;
		if (mask[i] == 0) continue;
		for (int k = 0; k < 3; k++)
			imgStd[k] += pow(lab[labIdx + k] - imgMean[k], 2);
	}

	for (int k = 0; k < 3; k++)
		imgStd[k] = sqrtf(imgStd[k] / pixCount);
	outMean = imgMean;
	outStd = imgStd;

	//SmartBody::util::log("Image Mean = %s, Std = %s", outMean.toString().c_str(), outStd.toString().c_str());
}

void imageColorTransfer(std::string srcImg, std::string srcMask, std::string tgtImg, std::string tgtMask, std::string outImage)
{
	stbi_set_flip_vertically_on_load(false);
	int srcWidth, srcHeight, srcChannel;
	int forceImgChannel = 4;
	int forceMaskChannel = 1;
	unsigned char* srcBuf = stbi_load(srcImg.c_str(), &srcWidth, &srcHeight, &srcChannel, forceImgChannel);
	unsigned char* srcMaskBuf = stbi_load(srcMask.c_str(), &srcWidth, &srcHeight, &srcChannel, forceMaskChannel);

	int tgtWidth, tgtHeight, tgtChannel;
	unsigned char* tgtBuf = stbi_load(tgtImg.c_str(), &tgtWidth, &tgtHeight, &tgtChannel, forceImgChannel);
	unsigned char* tgtMaskBuf = stbi_load(tgtMask.c_str(), &tgtWidth, &tgtHeight, &tgtChannel, forceMaskChannel);

	int srcSize = srcHeight*srcWidth;
	int tgtSize = tgtHeight*tgtWidth;
	float* srcLab = new float[srcSize*forceImgChannel];
	float* tgtLab = new float[tgtSize*forceImgChannel];

	rgb2lab(srcBuf, srcLab, srcSize*forceImgChannel);
	rgb2lab(tgtBuf, tgtLab, tgtSize*forceImgChannel);

	SrVec srcMean, srcStd, tgtMean, tgtStd;
	computeImageMeanAndStd(srcLab, srcMaskBuf, srcSize, srcMean, srcStd);
	computeImageMeanAndStd(tgtLab, tgtMaskBuf, tgtSize, tgtMean, tgtStd);

	SrVec newStdRato;
	for (int k = 0; k < 3; k++)
		newStdRato[k] = tgtStd[k] / srcStd[k];
	for (int i = 0; i < srcSize; i++)
	{
		if (srcMaskBuf[i] == 0) continue;
		int labIdx = i * 4;
		for (int k = 0; k < 3; k++)
		{
			// adjust source image in LAB space based on target image's Mean and Std
			srcLab[labIdx + k] = (srcLab[labIdx + k] - srcMean[k])*newStdRato[k] + tgtMean[k];
		}
	}
	lab2rgb(srcLab, srcBuf, srcSize*forceImgChannel);
	int imageWriteSuccess = stbi_write_png(outImage.c_str(), srcWidth, srcHeight, 4, srcBuf, srcWidth * 4);
	//SmartBody::util::log("Writing PNG %s, result = %d", outImage.c_str(), imageWriteSuccess);
}

