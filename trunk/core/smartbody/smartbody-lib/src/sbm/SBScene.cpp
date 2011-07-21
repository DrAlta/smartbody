#include "SBScene.h"
#include <sbm/mcontrol_util.h>

SBScene::SBScene(void)
{
	createBoolAttribute("internalAudio",false,true,"",10,false,false,false,"Use SmartBody's internal audio player.");
}

SBScene::~SBScene(void)
{
}

void SBScene::notify( DSubject* subject )
{
	BoolAttribute* boolAttr = dynamic_cast<BoolAttribute*>(subject);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (boolAttr && boolAttr->getName() == "internalAudio")
	{
		mcu.play_internal_audio = boolAttr->getValue();		
	}
}