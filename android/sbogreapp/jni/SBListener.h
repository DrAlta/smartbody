#ifndef _SBLISTENER_H
#define _SBLISTENER_H

#include <string>
#include <sbm/mcontrol_util.h>

class OgreFramework;

class SBListener : public SBCharacterListener
{
	public:
		SBListener(OgreFramework* app);		
		virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
		virtual void OnCharacterDelete( const std::string & name );
		virtual void OnCharacterChange( const std::string & name );

	protected:
		OgreFramework* m_app;
};


#endif
