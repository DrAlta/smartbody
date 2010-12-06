
#ifndef SMARTBODY_DLL_H
#define SMARTBODY_DLL_H


#ifdef SMARTBODY_DLL_EXPORTS
#define SMARTBODY_DLL_API __declspec(dllexport)
#else
#define SMARTBODY_DLL_API __declspec(dllimport)
#endif


#include "vhcl_public.h"
#include <queue>
#include <map>

// Listener class that executables should derive from to get Smartbody related notifications.
class SmartbodyListener
{
   public:
      virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass ) {}
      virtual void OnCharacterDelete( const std::string & name ) {}
      virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime ) {}
};


// helper class for receiving individual joint data
class SmartbodyJoint
{
   public:
      std::string m_name;
      float x;
      float y;
      float z;
      float rw;
      float rx;
      float ry;
      float rz;
};


// helper class for receiving character data including all the joints
class SmartbodyCharacter
{
   public:
      std::string m_name;
      float x;
      float y;
      float z;
      float rw;
      float rx;
      float ry;
      float rz;

      std::vector< SmartbodyJoint > m_joints;
};


class Smartbody_dll_SBMCharacterListener_Internal;

class Smartbody_dll
{
   private:
      SmartbodyListener * m_listener;
      Smartbody_dll_SBMCharacterListener_Internal * m_internalListener;

   public:
      SMARTBODY_DLL_API Smartbody_dll();
      SMARTBODY_DLL_API virtual ~Smartbody_dll();

      SMARTBODY_DLL_API void SetSpeechAudiofileBasePath( const std::string & basePath );
      SMARTBODY_DLL_API void SetFacebone( const bool enabled );
      SMARTBODY_DLL_API void SetProcessId( const std::string & processId );

      SMARTBODY_DLL_API bool Init();
      SMARTBODY_DLL_API bool Shutdown();

      SMARTBODY_DLL_API void SetListener( SmartbodyListener * listener );

      SMARTBODY_DLL_API bool Update( const double timeInSeconds );


      SMARTBODY_DLL_API bool ProcessVHMsgs( const char * op, const char * args );


      SMARTBODY_DLL_API int GetNumberOfCharacters();

      SMARTBODY_DLL_API SmartbodyCharacter GetCharacter( const std::string & name );

   protected:
      bool InitVHMsg();
      void RegisterCallbacks();

      friend Smartbody_dll_SBMCharacterListener_Internal;
};


//--------------------------------------------------------------------------
// Experimental interface 
// Can you be called from C# and only uses primitive input/output parameters
//--------------------------------------------------------------------------

struct SimpleViseme
{
	std::string name;
	std::string visemeName;
	float weight;
	float blendTime;
};

struct SimpleCharacter
{
	std::string name;
	std::string objectClass;
};

class SimpleSmartbodyListener : public SmartbodyListener
{
	public:
		SimpleSmartbodyListener();
		~SimpleSmartbodyListener();

		void OnCharacterCreate( const std::string & name, const std::string & objectClass );
		void OnCharacterDelete( const std::string & name );
		void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime );

		static SimpleSmartbodyListener* listener;

		Smartbody_dll* sbm;

		std::queue<SimpleCharacter> charactersCreated;
		std::queue<SimpleCharacter> charactersDeleted;
		std::map<std::string, std::queue<SimpleViseme> > visemes;
};

SMARTBODY_DLL_API bool HasCharacterCreated(std::string& name, std::string& objectClass);
SMARTBODY_DLL_API bool HasCharacterDeleted(std::string& name);
SMARTBODY_DLL_API bool HasViseme(const std::string & name, const std::string & visemeName, const float weight, const float blendTime);
SMARTBODY_DLL_API void SetSpeechAudiofileBasePath( const std::string & basePath );
SMARTBODY_DLL_API void SetFacebone( const bool enabled );
SMARTBODY_DLL_API void SetProcessId( const std::string & processId );
SMARTBODY_DLL_API void Init();
SMARTBODY_DLL_API void Shutdown();
SMARTBODY_DLL_API bool Update( const double timeInSeconds );
SMARTBODY_DLL_API bool ProcessVHMsgs( const char * op, const char * args );
SMARTBODY_DLL_API int GetNumberOfCharacters();
SMARTBODY_DLL_API void GetCharacterInfo(std::string name, float& x, float& y, float& z, float& rw, float& rx, float& ry, float& rz);
SMARTBODY_DLL_API void GetCharacterJointInfo(std::string name, int jointNum, std::string& jointName, float& x, float& y, float& z, float& rw, float& rx, float& ry, float& rz);
SMARTBODY_DLL_API int GetNumJoints(std::string name);

#endif  // SMARTBODY_DLL_H
