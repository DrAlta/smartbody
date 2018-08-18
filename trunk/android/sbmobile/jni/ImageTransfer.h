#ifndef __IMAGE_TRANSFER__
#define __IMAGE_TRANSFER__
#include <string>
#if __cplusplus
extern "C"
{
#endif
	void imageColorTransfer(std::string srcImg, std::string srcMask, std::string tgtImg, std::string tgtMask, std::string outImage);

#if __cplusplus
}
#endif
#endif