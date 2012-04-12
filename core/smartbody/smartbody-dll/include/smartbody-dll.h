
#ifndef SMARTBODY_DLL_H
#define SMARTBODY_DLL_H

#ifdef WIN32
#ifdef SMARTBODY_DLL_EXPORTS
#define SMARTBODY_DLL_API __declspec(dllexport)
#else
#define SMARTBODY_DLL_API __declspec(dllimport)
#endif
#else
#define SMARTBODY_DLL_API 
#endif

#include "vhcl_public.h"
#include <string>
#include <queue>
#include <map>


// Listener class that executables should derive from to get Smartbody related notifications.
class SmartbodyListener
{
   public:
      virtual void OnCharacterCreate( const std::string& name, const std::string& objectClass ) {}
      virtual void OnCharacterDelete( const std::string& name ) {}
      virtual void OnCharacterChanged( const std::string& name ) {}
      virtual void OnViseme( const std::string& name, const std::string& visemeName, const float weight, const float blendTime ) {}
	  virtual void OnChannel( const std::string& name, const std::string& channelName, const float value ) {}
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
class SbmDebuggerServer;

class Smartbody_dll
{
   private:
      SmartbodyCharacter m_emptyCharacter;
      SmartbodyListener * m_listener;
      Smartbody_dll_SBMCharacterListener_Internal * m_internalListener;
      std::map<std::string, SmartbodyCharacter*> m_characters;

   public:
      SMARTBODY_DLL_API Smartbody_dll();
      SMARTBODY_DLL_API virtual ~Smartbody_dll();

      SMARTBODY_DLL_API void SetSpeechAudiofileBasePath( const std::string & basePath );
      SMARTBODY_DLL_API void SetProcessId( const std::string & processId );
      SMARTBODY_DLL_API void SetMediaPath( const std::string & path );

      SMARTBODY_DLL_API bool Init(const std::string& pythonLibPath, bool logToFile);
      SMARTBODY_DLL_API bool Shutdown();

      SMARTBODY_DLL_API void SetListener( SmartbodyListener * listener );

      SMARTBODY_DLL_API bool Update( const double timeInSeconds );

      SMARTBODY_DLL_API void SetDebuggerId( const std::string & id );
      SMARTBODY_DLL_API void SetDebuggerCameraValues( double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar );
      SMARTBODY_DLL_API void SetDebuggerRendererRightHanded( bool enabled );

      SMARTBODY_DLL_API bool ProcessVHMsgs( const char * op, const char * args );

      SMARTBODY_DLL_API int GetNumberOfCharacters();

      SMARTBODY_DLL_API SmartbodyCharacter& GetCharacter( const std::string & name );

      SMARTBODY_DLL_API bool PythonCommandVoid( const std::string & command );
      SMARTBODY_DLL_API bool PythonCommandBool( const std::string & command );
      SMARTBODY_DLL_API int PythonCommandInt( const std::string & command );
      SMARTBODY_DLL_API float PythonCommandFloat( const std::string & command );
      SMARTBODY_DLL_API std::string PythonCommandString( const std::string & command );

   protected:
      bool InitVHMsg();
      void RegisterCallbacks();

      friend class Smartbody_dll_SBMCharacterListener_Internal;
};

#endif  // SMARTBODY_DLL_H
