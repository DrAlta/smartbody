#ifndef _OGRESMARTBODYLISTENER_H_
#define _OGRESMARTBODYLISTENER_H_

#include <ogre/Ogre.h>
#include "ogresmartbody.h"
#include <sb/SBCharacterListener.h>

class OgreSmartBodyListener : public SmartBody::SBCharacterListener
{
   public:
	   OgreSmartBodyListener(OgreSmartBody* osb);
	   ~OgreSmartBodyListener();

		virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );	
		virtual void OnCharacterDelete( const std::string & name );
		virtual void OnCharacterChanged( const std::string& name );		 
		virtual void OnLogMessage( const std::string & message );


	private:
		OgreSmartBody* ogreSB;
};

#endif