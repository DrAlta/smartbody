#ifndef _SBMPYTHONCLASS_
#define _SBMPYTHONCLASS_



#include "vhcl.h"
#include <sbm/mcontrol_util.h>
#include <sbm/resource_cmds.h>
#include <sbm/sbm_character.hpp>
#include <sbm/me_utilities.hpp>
#include <sk/sk_skeleton.h>
#include <sk/sk_joint.h>
#include <sbm/sbm_test_cmds.hpp>
#include <map>
#include <sbm/SBJoint.h>
#include <sbm/SBSkeleton.h>
#include <sbm/SBCharacter.h>
#include <sbm/SBController.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#if !defined (__ANDROID__) && !defined(SBM_IPHONE)
#ifndef USE_PYTHON
#define USE_PYTHON
#endif
#endif

#ifdef USE_PYTHON
#include <boost/python.hpp>

namespace SmartBody 
{


class PyLogger
{
protected:
	static std::string strBuffer;
public:
	static void pa();
	static void pb();	
	static void pc();
	static void pd();	
	static void pe();
	static void pf();	
	static void pg();
	static void ph();	
	static void pi();
	static void pj();	
	static void pk();
	static void pl();	
	static void pm();
	static void pn();	
	static void po();
	static void pp();	
	static void pq();
	static void pr();	
	static void ps();
	static void pt();	
	static void pu();
	static void pv();	
	static void pw();
	static void px();	
	static void py();
	static void pz();	
	static void p1();
	static void p2();
	static void p3();
	static void p4();
	static void p5();
	static void p6();
	static void p7();
	static void p8();
	static void p9();
	static void p0();

	static void openparen() { strBuffer += "("; }
	static void closeparen() { strBuffer += ")"; }
	static void openbracket() { strBuffer += "["; }
	static void closebracket() { strBuffer += "]"; }
	static void openbrace() { strBuffer += "{"; }
	static void closebrace() { strBuffer += "}"; }
	static void plus() { strBuffer += "+"; }
	static void minus() { strBuffer += "-"; }
	static void aster() { strBuffer += "*"; }
	static void slash() { strBuffer += "\\"; }
	static void backslash() { strBuffer += "/"; }
	static void comma() { strBuffer += ","; }
	static void colon() { strBuffer += ":"; }
	static void semicolon() { strBuffer += ";"; }
	static void equal() { strBuffer += "="; }
	static void less() { strBuffer += "<"; }
	static void more() { strBuffer += ">"; }

	static void pspace();
	static void pnon();
	static void outlog();
};

class Camera
{
	public:
		Camera();
		~Camera();
		
		void printInfo();

		void reset();
		void setDefault(int preset = 1);

		void setEye(float x = 0.0, float y = 0.0, float z = 0.0);
		void setCenter(float x = 0.0, float y = 0.0, float z = 0.0);
		void setScale(float s = 1.0);

		void setTrack(std::string cName, std::string jName);
		void removeTrack();
};

class Script
{
	public:
		Script();
		Script(std::string seqName);
		~Script();

		void print();
		void run();
		void abort();

		void setType(const std::string& typ) {type = typ;}
		const std::string& getType()		{return type;}

	protected:
		std::string seq;
		void preProcessingScript(srCmdSeq *to_seq_p, srCmdSeq *fr_seq_p);
		std::string type;
};




class Viseme
{
	public:
		Viseme();
		~Viseme();
		void setVisemeName(const std::string& name) {visemeName = name;}
		const std::string& getVisemeName() {return visemeName;}
		void setCharName(const std::string& name) {charName = name;}
		const std::string& getCharName() {return charName;}

		void setWeight(float weight, float dur, float rampin, float rampout);
		void setCurve(int num, boost::python::list weights);

	protected:
		std::string visemeName;
		std::string charName;
};

class Motion
{
	public:
		Motion();
		Motion(std::string motionFile);
		~Motion();

		const std::string& getMotionFileName();
		const std::string& getMotionName();
		int getNumFrames();
		boost::python::list getFrameData(int i);
		int getFrameSize();

		int getNumChannel();
		boost::python::list getChannels();
		void checkSkeleton(std::string skel);

		SkMotion* getSkMotion();
		
		void connect(SBSkeleton* skel);
		void disconnect();

	protected:
		std::string motionFile;
		std::string emptyString;
};

// SmartBody functions


void pythonExit();
void quitSbm();
void reset();
SBScene* getScene();

void printLog(const std::string& message);

SBController* createController(std::string controllerType, std::string controllerName);

Camera* getCamera();
SrViewer* getViewer();
GenericViewer* getBmlViewer();
GenericViewer* getDataViewer();

void execScripts(boost::python::list& input);
Script* getScript(std::string fileName);
void showCommandResources();
void showMotionResources();
void showSkeletonResources();
void showPathResources();
void showScriptResources();
void showControllerResources();
void getResourceLimit();
void setResourceLimit(int limit);









// helper functions
std::string getScriptFromFile(std::string fileName);

class PythonController :  public SBController
{
public:
	std::string controllerType;
	PythonController() : SBController() { controllerType = "python";}
	virtual void start() {};
	virtual void init() {};
	virtual void evaluate() {};
	virtual void stop() {};

	virtual SkChannelArray& controller_channels () { return channels;}
	virtual double controller_duration () { return  1000000.0; }
	const std::string& controller_type() const { return controllerType; }
	bool controller_evaluate(double t, MeFrameData& frame )
	{
		evaluate();
		return true;
	}

protected:
	SkChannelArray channels;
};


}
#endif

#endif
