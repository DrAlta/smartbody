#ifndef _SBBEHAVIOR_H_
#define _SBBEHAVIOR_H_

#include <string>
#include <sr/sr_vec.h>

namespace SmartBody {

class SBBehavior
{
	public:
		SBBehavior();
		~SBBehavior();

		virtual const std::string& getType();

	protected:
		std::string _type;

};


class SpeechBehavior : public SBBehavior
{
	public:
		SpeechBehavior();
		~SpeechBehavior();

		virtual void setUtterance(const std::string& utterance);
		virtual const std::string& getUtterance();

	protected:
		std::string _utterance;


};

class GazeBehavior : public SBBehavior
{
	public:
		GazeBehavior();
		~GazeBehavior();

		virtual void setGazeTarget(const std::string& target);
		virtual const std::string& getGazeTarget();
		void setFadingIn(bool val);
		void setFadingOut(bool val);
		void setFadedOut(bool val);
		bool isFadingIn();
		bool isFadingOut();
		bool isFadedOut();
		void setHandle(const std::string& handle);
		const std::string& getHandle();

	protected:
		std::string _target;
		bool _fadingIn;
		bool _fadingOut;
		bool _fadedOut;
		std::string _handle;

};

class LocomotionBehavior : public SBBehavior
{
	public:
		LocomotionBehavior();
		~LocomotionBehavior();

		virtual void setLocomotionTarget(const SrVec& target);
		virtual const SrVec& getLocomotionTarget();

	protected:
		SrVec _target;


};
};

#endif