#include "RealTimeLipSyncController.h"
#include <sb/SBScene.h>
#include <sb/SBMotion.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBPhoneme.h>

RealTimeLipSyncController::RealTimeLipSyncController(SmartBody::SBCharacter* c) : SmartBody::SBController()
{
	_pawn = c;
	setup();
}

RealTimeLipSyncController::RealTimeLipSyncController() : SmartBody::SBController()
{
	setup();
}

void RealTimeLipSyncController::setup()
{
	// breathing settings
	setDefaultAttributeGroupPriority("Lip Sync", 450);
	addDefaultAttributeBool("lipsync.useRealTimeLipSync", false, "Lip Sync");
	addDefaultAttributeString("lipsync.realTimeLipSyncName", "", "Lip Sync");
}

void RealTimeLipSyncController::updateLipSyncChannels()
{
	if (!_pawn)
		return;

	SmartBody::StringAttribute* attribute = dynamic_cast<SmartBody::StringAttribute*>(_pawn->getAttribute("lipsyncSetName"));
	if (attribute)
	{
		const std::string& value = attribute->getValue();
		std::vector<SmartBody::SBDiphone*>& allDiphones = SmartBody::SBScene::getScene()->getDiphoneManager()->getDiphones(value);
		std::set<std::string> allVisemes;
		for (size_t d = 0; d < allDiphones.size(); d++)
		{
			std::vector<std::string> visemes = allDiphones[d]->getVisemeNames();
			for (size_t v = 0; v < visemes.size(); v++)
			{
				allVisemes.insert(visemes[v]);
			}
		}
	}
}

RealTimeLipSyncController::~RealTimeLipSyncController()
{
}

void RealTimeLipSyncController::init(SmartBody::SBPawn* pawn)
{
	MeController::init(pawn);

	_lastPhonemeTime = 0.0;
	_lastPhoneme = "";

	SmartBody::SBAttribute* attribute = _pawn->getAttribute("lipsyncSetName");
	if (!attribute)
		return;
	// make sure that the channel list is updated if the set is updated
	attribute->registerObserver(this);

	// determine the channels to be used for lip syncing
	const std::string& lipSyncSetName = _pawn->getStringAttribute("lipsyncSetName");

}

bool RealTimeLipSyncController::controller_evaluate ( double t, MeFrameData& frame )
{
	if (!_pawn)
		return false;

	if (!_pawn->getBoolAttribute("lipsync.useRealTimeLipSync"))
		return false;

	if (_motion)
	{
		double curTime = t - _startTime;
		if (curTime > _motion->getDuration())
		{
			delete _motion;
			_motion = NULL;
		}
		else
		{
			// apply the motion to the frame
			// ...
		}
	}

	SmartBody::SBPhonemeManager* phonemeManager = SmartBody::SBScene::getScene()->getDiphoneManager();

	const std::string& realTimeLipSyncName = _pawn->getStringAttribute("lipsync.realTimeLipSyncName");

	const std::string& lipSyncSetName = _pawn->getStringAttribute("lipsyncSetName");
	std::vector<std::string> phonemeList = phonemeManager->getPhonemesRealtime(realTimeLipSyncName);
	std::vector<double> phonemeTimingsList = phonemeManager->getPhonemesRealtimeTimings(realTimeLipSyncName);

	for (size_t p = 0; p < phonemeTimingsList.size(); p++)
	{
		_currentPhonemes.push_back(phonemeList[p]);
		_currentPhonemeTimings.push_back(phonemeTimingsList[p]);
	}

	if (_currentPhonemes.size() >= 2)
	{
		if (_motion)
		{
			delete _motion;
			_motion = new SmartBody::SBMotion();
		}

		double fromTime = _currentPhonemeTimings.size() - 2;
		double toTime = _currentPhonemeTimings.size() - 1;

		double diphoneInterval = toTime - fromTime;
		if (diphoneInterval <= 0.0)
		{
			_currentPhonemes.clear();
			_currentPhonemeTimings.clear();
			return true;
		}

		std::string fromPhoneme = _currentPhonemes[_currentPhonemes.size() - 2];
		std::string toPhoneme = _currentPhonemes[_currentPhonemes.size() - 1];

		SmartBody::SBDiphone* diphone = phonemeManager->getDiphone(fromPhoneme, toPhoneme, lipSyncSetName);
		if (diphone)
		{
			double diphoneTiming = toTime - fromTime;
			
			if (diphoneTiming > 0.0)
			{
				// rescale the curves according to the timing and start playing them	
				std::vector<std::string> diphoneVisemes = diphone->getVisemeNames();
				for (size_t v = 0; v < diphoneVisemes.size(); v++)
				{
					std::vector<float>& diphoneKeys = diphone->getKeys(diphoneVisemes[v]);
					for (size_t k = 0; k < diphoneKeys.size(); k++)
					{
						_motion->addKeyFrameChannel(diphoneVisemes[v], "XPos", diphoneKeys[k], diphoneKeys[k + 1]);
					}
				}
				_motion->bakeFrames(60.0);
			}
		}
	}

	_currentPhonemes.clear();
	_currentPhonemeTimings.clear();

	return true;
}


void RealTimeLipSyncController::notify(SmartBody::SBSubject* subject)
{
	SBController::notify(subject);

	SmartBody::SBAttribute* attribute = dynamic_cast<SmartBody::SBAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "lipsyncSetName")
		{
			updateLipSyncChannels();
		}
	}



}


