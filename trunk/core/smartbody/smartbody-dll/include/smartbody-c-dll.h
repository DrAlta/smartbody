
#ifndef SMARTBODY_C_DLL_H
#define SMARTBODY_C_DLL_H


#ifdef SMARTBODY_C_DLL_EXPORTS
#define SMARTBODY_C_DLL_API __declspec(dllexport)
#else
#define SMARTBODY_C_DLL_API __declspec(dllimport)
#endif


#include "vhcl_public.h"

#include "smartbody-dll.h"


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

SMARTBODY_C_DLL_API bool HasCharacterCreated(std::string& name, std::string& objectClass);
SMARTBODY_C_DLL_API bool HasCharacterDeleted(std::string& name);
SMARTBODY_C_DLL_API bool HasViseme(const std::string & name, const std::string & visemeName, const float weight, const float blendTime);
SMARTBODY_C_DLL_API void SetSpeechAudiofileBasePath( const std::string & basePath );
SMARTBODY_C_DLL_API void SetFacebone( const bool enabled );
SMARTBODY_C_DLL_API void SetProcessId( const std::string & processId );
SMARTBODY_C_DLL_API void Init();
SMARTBODY_C_DLL_API void Shutdown();
SMARTBODY_C_DLL_API bool Update( const double timeInSeconds );
SMARTBODY_C_DLL_API bool ProcessVHMsgs( const char * op, const char * args );
SMARTBODY_C_DLL_API int GetNumberOfCharacters();
SMARTBODY_C_DLL_API void GetCharacterInfo(std::string name, float& x, float& y, float& z, float& rw, float& rx, float& ry, float& rz);
SMARTBODY_C_DLL_API void GetCharacterJointInfo(std::string name, int jointNum, std::string& jointName, float& x, float& y, float& z, float& rw, float& rx, float& ry, float& rz);
SMARTBODY_C_DLL_API int GetNumJoints(std::string name);

#endif  // SMARTBODY_C_DLL_H
