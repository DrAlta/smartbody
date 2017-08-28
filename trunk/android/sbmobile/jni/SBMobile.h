#ifndef __SB_MOBILE__
#define __SB_MOBILE__
#include <vector>
#include <string>
#include <sr/sr_vec.h>
#include <sb/SBTypes.h>
#include <jni.h>

void initSBMobilePythonModule();

class SBMobile
{
public:
	SBMobile();
	~SBMobile();

		
	// Video API
	void playVideo(std::string videoViewName, std::string videoFilePath, bool looping = false);
	void stopVideo(std::string videoViewName);
	
	// Sound API
	void playSound(std::string soundFilePath, bool looping = false);
	void stopSound();



	// Handle screen touch event
	void resize(int w, int h);
	SrVec convertScreenSpaceTo3D(float x, float y, SrVec ground, SrVec upVector);
	std::string testCharacterIntersection(float x, float y, std::string charName);

	virtual bool eventScreenTouch(int action, float x, float y);

	void snapshotPNGResize(std::string imgFileName, int width, int height, int outW = -1, int outH = -1);
	
	void callJNIMethod(std::string className, std::string methodName, std::string methodDef, std::vector<std::string> parameters);		
	bool beforeCallJavaMethod( const std::string& className, const std::string& methodName, const std::string& methodDef, jclass& interfaceClass, jmethodID& method);	

	static SBMobile* getSBMobile() { return engine; }
	static void setSBMobile(SBMobile* inEngine) { engine = inEngine; }

	static JavaVM* jvm;
	static JNIEnv* env;
	static bool jvmIsAttached;
	static SBMobile* engine;
	
protected:
	int screenWidth, screenHeight;
	jstring stringToJString(const std::string& str);
	
};



#endif
