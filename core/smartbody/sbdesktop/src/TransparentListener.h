#ifndef _TRANSPARENTLISTENER_H_
#define _TRANSPARENTLISTENER_H_

#include <sbm/mcontrol_util.h>
#include <sk/sk_scene.h>

class TransparentListener : public SmartBody::SBCharacterListener, public SmartBody::SBObserver
{
   public:
	  TransparentListener();
	  ~TransparentListener();

      virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
      virtual void OnCharacterDelete( const std::string & name );
	  virtual void OnCharacterUpdate( const std::string & name, const std::string & objectClass );
	  virtual void OnCharacterChanged( const std::string& name );
      virtual void OnPawnCreate( const std::string & name );
      virtual void OnPawnDelete( const std::string & name );
      virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime );
	  virtual void OnChannel( const std::string & name, const std::string & channelName, const float value);
	  virtual void OnReset();

	  virtual void notify(SmartBody::SBSubject* subject);
};

#endif