#ifndef _IRRLICHTSMARTBODYLISTENER_H_
#define _IRRLICHTSMARTBODYLISTENER_H_

#include <irrlicht.h>
#include <sb/SBCharacterListener.h>
#include <ISceneManager.h>
#include <map>

class IrrlichtSmartBodyListener : public SmartBody::SBCharacterListener
{
   public:
	   IrrlichtSmartBodyListener(irr::scene::ISceneManager* sceneMgr, std::map<std::string, int>* characterMap);
	   ~IrrlichtSmartBodyListener();

		virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );	
		virtual void OnCharacterDelete( const std::string & name );
		virtual void OnCharacterChanged( const std::string& name );		 
		virtual void OnLogMessage( const std::string & message );


	private:
		irr::scene::ISceneManager* mSceneMgr;
		std::map<std::string, int>* map;
		int id;
};

#endif