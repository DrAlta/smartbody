#pragma once
#include <sbm/DObject.h>
#include <sbm/DSubject.h>

class TreeInfoObject : public DObject
{
public:
	TreeInfoObject(void);
	~TreeInfoObject(void);

	virtual void notify(DSubject* subject);
};
