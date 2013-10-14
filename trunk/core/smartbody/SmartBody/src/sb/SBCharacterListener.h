#ifndef  __SBM_CHARACTER_LISTENER__
#define  __SBM_CHARACTER_LISTENER__
#include <string>
#include <sb/SBTypes.h>

namespace SmartBody
{
// This class is meant for listening to specific events that could be handled externally from smartbody
// Currently being used by smartbody-dll
class SBCharacterListener
{
public:
	SBAPI SBCharacterListener() {}
	virtual SBAPI ~SBCharacterListener() {}
	virtual SBAPI void OnCharacterCreate( const std::string & name, const std::string & objectClass ) {}
	virtual SBAPI void OnCharacterDelete( const std::string & name ) {}
	virtual SBAPI void OnCharacterUpdate( const std::string & name ) {}
	virtual SBAPI void OnPawnCreate( const std::string & name ) {}
	virtual SBAPI void OnPawnDelete( const std::string & name ) {}
	virtual SBAPI void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime ) {}
	virtual SBAPI void OnChannel( const std::string & name, const std::string & channelName, const float value) {}
	virtual SBAPI void OnLogMessage( const std::string & message) {}
};

}

#endif
