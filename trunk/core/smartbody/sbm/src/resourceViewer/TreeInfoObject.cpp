#include "TreeInfoObject.h"

TreeInfoObject::TreeInfoObject(void)
{
}

TreeInfoObject::~TreeInfoObject(void)
{
}

void TreeInfoObject::notify( DSubject* subject )
{
	DAttribute* attribute = dynamic_cast<DAttribute*>(subject);
	if (attribute)
	{
		//constructBML();
		notifyObservers();
	}
}