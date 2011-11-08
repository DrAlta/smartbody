#pragma once
#include <sbm/SBObject.h>
#include <sbm/SBSubject.h>

class TreeInfoObject : public SmartBody::SBObject
{
public:
	TreeInfoObject(void);
	~TreeInfoObject(void);

	virtual void notify(SmartBody::SBSubject* subject);
};
