#ifndef _SBLISTENER_H
#define _SBLISTENER_H

#include "smartbody-dll.h"
#include <string>

class PandaRenderer;

class SBListener : public SmartbodyListener
{
	public:
		SBListener(PandaRenderer* app);

		virtual void OnCharacterCreate( const std::string & name );
		virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
		virtual void OnCharacterDelete( const std::string & name );
		virtual void OnCharacterChange( const std::string & name );

	protected:
		PandaRenderer* m_app;


};


#endif