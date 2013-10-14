#ifndef _FLTKLISTENER_H_
#define _FLTKLISTENER_H_

#include "sb/SBCharacterListener.h"
#include "sb/SBObserver.h"

class FLTKListener : public SmartBody::SBCharacterListener, public SmartBody::SBObserver
{
   public:
	  FLTKListener();
	  ~FLTKListener();

      virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
      virtual void OnCharacterDelete( const std::string & name );
	  virtual void OnCharacterUpdate( const std::string & name );
      virtual void OnPawnCreate( const std::string & name );
      virtual void OnPawnDelete( const std::string & name );
      virtual void OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime );
	  virtual void OnChannel( const std::string & name, const std::string & channelName, const float value);
	  virtual void OnReset();
	  virtual void OnLogMessage( const std::string & message );

	  virtual void OnObjectSelected(const std::string& objectName);

	  virtual void notify(SmartBody::SBSubject* subject);

	  void setOtherListener(SBCharacterListener* listener);
	protected:
	  SmartBody::SBCharacterListener* otherListener;
};

#endif
