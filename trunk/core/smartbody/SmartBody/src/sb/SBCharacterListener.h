#ifndef  __SBM_CHARACTER_LISTENER__
#define  __SBM_CHARACTER_LISTENER__
#include <string>

namespace SmartBody
{
// This class is meant for listening to specific events that could be handled externally from smartbody
// Currently being used by smartbody-dll
class SBCharacterListener
{
public:
	virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass ) {}
	virtual void OnCharacterDelete( const std::string & name ) {}
	virtual void OnCharacterUpdate( const std::string & name, const std::string & objectClass ) {}
	virtual void OnCharacterChanged( const std::string& name ) {}
	virtual void OnCharacterChangeMesh( const std::string& name ) {}
	virtual void OnPawnCreate( const std::string & name ) {}
	virtual void OnPawnDelete( const std::string & name ) {}
	virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime ) {}
	virtual void OnChannel( const std::string & name, const std::string & channelName, const float value) {}
	virtual void OnLogMessage( const std::string & message) {}
};

}

#endif
