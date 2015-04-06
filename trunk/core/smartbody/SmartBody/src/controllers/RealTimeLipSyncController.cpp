#include "RealTimeLipSyncController.h"
#include <sb/SBScene.h>
#include <sb/SBMotion.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBPhoneme.h>
#include <bml/bml_speech.hpp>

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

	// _currentPhonemes and _currentPhonemeTimings contain a list phonemes that are
	//    gathered from the real time phoneme manager
	// _currentCurves contain the animation data that should be played
	// the algorithm converts the phonemes+timings into animation curves

	// remove any curves that are no longer valid
	if (_currentCurves.size() > 0)
	{
		bool removeCurves = true;
		while (removeCurves)
		{
			removeCurves = false;
			for (std::map<std::string, srLinearCurve* >::iterator iter = _currentCurves.begin();
				 iter != _currentCurves.end();
				 iter++)
			{
				srLinearCurve* curve = (*iter).second;
				double lastKeyTime = curve->get_tail_param();
				if (lastKeyTime + 100.0 < t)
				{
					delete curve;
					_currentCurves.erase(iter);
					removeCurves = true;
					break;
				}
			}
		}
	}

	if (_currentCurves.size() > 0)
	{
		for (std::map<std::string, srLinearCurve* >::iterator iter = _currentCurves.begin();
			 iter != _currentCurves.end();
			 iter++)
		{
			const std::string& visemeName = (*iter).first;
			srLinearCurve* curve = (*iter).second;

			double value = curve->evaluate(t);
			if (value > 0.0)
			{
				this->setChannelValue(visemeName, value);
				LOG("%s %f", visemeName.c_str(), value);
			}
		}
	}
	
	SmartBody::SBPhonemeManager* phonemeManager = SmartBody::SBScene::getScene()->getDiphoneManager();

	const std::string& realTimeLipSyncName = _pawn->getStringAttribute("lipsync.realTimeLipSyncName");

	const std::string& lipSyncSetName = _pawn->getStringAttribute("lipSyncSetName");
	std::vector<std::string> phonemeList = phonemeManager->getPhonemesRealtime(realTimeLipSyncName);
	std::vector<double> phonemeTimingsList = phonemeManager->getPhonemesRealtimeTimings(realTimeLipSyncName);
	if (phonemeTimingsList.size() > 0)
		phonemeManager->removePhonemesRealtime(realTimeLipSyncName);

	for (size_t p = 0; p < phonemeTimingsList.size(); p++)
	{
		_currentPhonemes.push_back(phonemeList[p]);
		_currentPhonemeTimings.push_back(phonemeTimingsList[p]);
	}

	if (_currentPhonemes.size() >= 2)
	{
		// since we now have at least two phonemes, remove the existing animation curves
		// this probably shouldn't be done like this...would be better to fade out the current curves
		// rather than to remove them entirely
		for (std::map<std::string, srLinearCurve* >::iterator iter = _currentCurves.begin();
			 iter != _currentCurves.end();
			 iter++)
		{
			delete (*iter).second;
		}
		_currentCurves.clear();

		double fromTime = _currentPhonemeTimings[_currentPhonemeTimings.size() - 2];
		double toTime = _currentPhonemeTimings[_currentPhonemeTimings.size() - 1];

		double diphoneInterval = toTime - fromTime;
		if (diphoneInterval <= 0.0)
		{
			// if the timings are improper (end time > start time) 
			// then assume this data is bad, and remove the phonemes from the list
			_currentPhonemes.clear();
			_currentPhonemeTimings.clear();
			return true;
		}

		std::string fromPhoneme = _currentPhonemes[_currentPhonemes.size() - 2];
		std::string toPhoneme = _currentPhonemes[_currentPhonemes.size() - 1];
		
		SmartBody::VisemeData* visemeStart = new SmartBody::VisemeData(fromPhoneme, 0);
		SmartBody::VisemeData* visemeEnd = new SmartBody::VisemeData(toPhoneme, (float) diphoneInterval);
		std::vector<SmartBody::VisemeData*> visemes;
		visemes.push_back(visemeStart);
		visemes.push_back(visemeEnd);

		_lastPhonemeTime = t;
		// obtain a set of curves from the phoneme manager based on the two phonemes
		std::map<std::string, std::vector<float> > lipSyncCurves = BML::SpeechRequest::generateCurvesGivenDiphoneSet(&visemes, lipSyncSetName, _pawn->getName());

		for (std::map<std::string, std::vector<float> >::iterator iter = lipSyncCurves.begin();
			 iter != lipSyncCurves.end();
			 iter++)
		{
			srLinearCurve* curve = new srLinearCurve();
			for (std::vector<float>::iterator fiter = (*iter).second.begin();
				 fiter != (*iter).second.end();
				 fiter++)
			{
				float time = (*fiter);
				fiter++;
				float value = (*fiter);
				curve->insert(time + _lastPhonemeTime, value);
			}
			_currentCurves.insert(std::pair<std::string, srLinearCurve*>((*iter).first, curve));
			
		}

		// since we have converted the phonemes into animation curves, remove the phonemes from the list
		_currentPhonemes.clear();
		_currentPhonemeTimings.clear();
	}



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


