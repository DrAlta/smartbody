/*
 *  SBFaceDefinition.h - part of SmartBody-lib
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

#ifndef _SBFACEDEFINITION_H_
#define _SBFACEDEFINITION_H_

#include <sb/SBTypes.h>
#include <map>
#include <vector>
#include <string>
#include <sb/SBObject.h>

/**
 *  Maps viseme ids to the related motion.
 */
//typedef std::map< std::string, SkMotion* > VisemeMotionMap;

class ActionUnit;
class SkMotion;

namespace SmartBody {

class SBMotion;

class SBFaceDefinition : public SBObject
{
	public:
		SBAPI SBFaceDefinition();
		SBAPI SBFaceDefinition(const std::string& name);
		SBAPI SBFaceDefinition(SBFaceDefinition* source);

		SBAPI ~SBFaceDefinition();

		SBAPI void setFaceNeutral(const std::string& motionName);
		SBAPI SkMotion* getFaceNeutral();

		SBAPI bool hasViseme(const std::string& visemeName);
		SBAPI void setViseme(const std::string& visemeName, const std::string& motionName);
		SBAPI void setVisemeWeight(const std::string& visemeName, float weight);
		SBAPI int getNumVisemes();
		SBAPI std::vector<std::string> getVisemeNames();
		SBAPI const std::string& getVisemeName(int index);
		SBAPI SkMotion* getVisemeMotion(const std::string& viseme);
		SBAPI float getVisemeWeight(const std::string& viseme);


		SBAPI bool hasAU(int auNum);
		SBAPI ActionUnit* getAUByName(const std::string& auName);
		SBAPI void setAU(int auNum, const std::string& side, const std::string& motion);
		SBAPI int getNumAUs();
		SBAPI int getNumAUChannels();
		SBAPI int getAUNum(int index);
		SBAPI std::vector<int> getAUNumbers();
		SBAPI SBMotion* getAUMotion(int num, std::string side);
		SBAPI ActionUnit* getAU(int index);
		SBAPI std::string getAUSide(int num);

		SBAPI std::string saveToString();
		SBAPI void save(const std::string& fileName);
		
	protected:
		void addAU(int auNum, ActionUnit* au);
		SkMotion* _faceNeutral;

		std::map<int, ActionUnit*> _auMap;
		std::map<std::string, std::pair<SkMotion*, float> > _visemeMap;
		std::string _name;
		std::string _emptyString;
};

}

#endif // SBM_VISEME_MAP_HPP
