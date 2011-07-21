#ifndef SBSCENE_H
#define SBSCENE_H
#include <sbm/DObject.h>

class SBScene : public DObject
{
public:
	SBScene(void);
	~SBScene(void);

	void notify(DSubject* subject);
};

#endif