#pragma once

#include <sb/SBController.h>
#include <sb/SBCharacter.h>
#include <sb/SBMotion.h>
#include <sbm/sr_linear_curve.h>

class RealTimeLipSyncController : public SmartBody::SBController
{
	public:
		RealTimeLipSyncController(SmartBody::SBCharacter* character);
		RealTimeLipSyncController();
		~RealTimeLipSyncController();

		void init(SmartBody::SBPawn* pawn);

		virtual bool controller_evaluate ( double t, MeFrameData& frame );

		virtual void notify(SmartBody::SBSubject* subject);

	protected:
		void updateLipSyncChannels();
		void setup();
		double _lastPhonemeTime;
		std::string _lastPhoneme;
		std::vector<double> _currentPhonemeTimings;
		std::vector<std::string> _currentPhonemes;
		double _startTime;
		SkChannelArray  _channels;

		std::map<std::string, srLinearCurve* > _currentCurves;
};
		
