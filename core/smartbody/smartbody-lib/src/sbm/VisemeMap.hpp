/*
 *  viseme_map.hpp - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Andrew n marshall, USC
 */

#ifndef SBM_VISEME_MAP_HPP
#define SBM_VISEME_MAP_HPP

#include <map>
#include <sk/sk_motion.h>
#include "action_unit.hpp"

/**
 *  Maps viseme ids to the related motion.
 */
typedef std::map< std::string, SkMotion* > VisemeMotionMap;


class FaceDefinition
{
	public:
		FaceDefinition();
		FaceDefinition(const std::string& name);
		FaceDefinition(FaceDefinition* source);

		~FaceDefinition();

		const std::string& getName();
		void setName(const std::string& name);

		void setFaceNeutral(const std::string& motionName);
		SkMotion* getFaceNeutral();

		bool hasViseme(const std::string& visemeName);
		void setViseme(const std::string& visemeName, const std::string& motionName);
		void setVisemeWeight(const std::string& visemeName, float weight);
		int getNumVisemes();
		const std::string& getVisemeName(int index);
		SkMotion* getVisemeMotion(const std::string& viseme);
		float getVisemeWeight(const std::string& viseme);


		bool hasAU(int auNum);
		void setAU(int auNum, const std::string& side, const std::string& motion);
		int getNumAUs();
		int getNumAUChannels();
		int getAUNum(int index);
		ActionUnit* getAU(int index);
		
	protected:
		void addAU(int auNum, ActionUnit* au);
		SkMotion* _faceNeutral;

		std::map<int, ActionUnit*> _auMap;
		std::map<std::string, std::pair<SkMotion*, float> > _visemeMap;
		std::string _name;
		std::string _emptyString;
};

#endif // SBM_VISEME_MAP_HPP