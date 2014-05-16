
#ifndef SMARTBODY_DLL_H
#define SMARTBODY_DLL_H

#include "vhcl_public.h"

#include <string>
#include <queue>
#include <map>

#include "sb/SBTypes.h"


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


class Smartbody_dll_SBSceneListener_Internal;
class SBDebuggerServer;

class Smartbody_dll
{
   private:
      SmartbodyCharacter m_emptyCharacter;
      Smartbody_dll_SBSceneListener_Internal * m_internalListener;
      std::map<std::string, SmartbodyCharacter*> m_characters;

   public:
      SBAPI Smartbody_dll();
      SBAPI virtual ~Smartbody_dll();

      SBAPI bool Init(const std::string& pythonLibPath, bool logToFile);
      SBAPI bool Shutdown();

      SBAPI bool LoadSkeleton( const void * data, int sizeBytes, const char * skeletonName );
      SBAPI bool LoadMotion( const void * data, int sizeBytes, const char * motionName );

      SBAPI bool Update( const double timeInSeconds );

      SBAPI void SetDebuggerId( const std::string & id );
      SBAPI void SetDebuggerCameraValues( double x, double y, double z, double rx, double ry, double rz, double rw, double fov, double aspect, double zNear, double zFar );
      SBAPI void SetDebuggerRendererRightHanded( bool enabled );

      SBAPI bool ProcessVHMsgs( const char * op, const char * args );
      SBAPI bool ExecutePython( const char * command);

      SBAPI SmartbodyCharacter& GetCharacter( const std::string & name );

      SBAPI bool PythonCommandVoid( const std::string & command );
      SBAPI bool PythonCommandBool( const std::string & command );
      SBAPI int PythonCommandInt( const std::string & command );
      SBAPI float PythonCommandFloat( const std::string & command );
      SBAPI std::string PythonCommandString( const std::string & command );

   protected:
      bool InitVHMsg();
      void InitLocalSpeechRelay();

      friend class Smartbody_dll_SBSceneListener_Internal;
};

#endif  // SMARTBODY_DLL_H
